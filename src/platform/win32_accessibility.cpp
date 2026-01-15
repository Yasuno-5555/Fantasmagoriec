#include <windows.h>
#include <uiautomation.h>
#include <vector>
#include <string>
#include <map>
#include "core/contexts_internal.hpp"
#include "core/contexts_internal.hpp"

namespace fanta {
namespace internal {

// Individual Element Provider
class FantaElementProvider : public IRawElementProviderSimple {
public:
    FantaElementProvider(ID element_id, HWND hwnd) 
        : m_ref_count(1), m_id(element_id), m_hwnd(hwnd) {}

    // IUnknown methods
    IFACEMETHODIMP QueryInterface(REFIID riid, void** ppv) {
        if (riid == IID_IUnknown || riid == IID_IRawElementProviderSimple) {
            *ppv = static_cast<IRawElementProviderSimple*>(this);
        } else {
            *ppv = nullptr;
            return E_NOINTERFACE;
        }
        AddRef();
        return S_OK;
    }

    IFACEMETHODIMP_(ULONG) AddRef() { return InterlockedIncrement(&m_ref_count); }
    IFACEMETHODIMP_(ULONG) Release() {
        long res = InterlockedDecrement(&m_ref_count);
        if (res == 0) delete this;
        return res;
    }

    // IRawElementProviderSimple methods
    IFACEMETHODIMP get_ProviderOptions(ProviderOptions* pRetVal) {
        *pRetVal = ProviderOptions_ServerSideProvider;
        return S_OK;
    }

    IFACEMETHODIMP GetPatternProvider(PATTERNID patternId, IUnknown** pRetVal) {
        *pRetVal = nullptr;
        // Future: Implement Value/Invoke patterns for sliders/buttons
        return S_OK;
    }

    IFACEMETHODIMP GetPropertyValue(PROPERTYID propertyId, VARIANT* pRetVal) {
        pRetVal->vt = VT_EMPTY;
        
        auto const& store = GetStore();
        auto const& snap = store.accessibility_snapshot;
        
        const AccessibilityItem* target = nullptr;
        for (auto const& item : snap.items) {
            if (item.id == m_id) {
                target = &item;
                break;
            }
        }
        
        if (!target) return S_OK;

        if (propertyId == UIA_NamePropertyId) {
            pRetVal->vt = VT_BSTR;
            std::wstring wname(target->name.begin(), target->name.end());
            pRetVal->bstrVal = SysAllocString(wname.c_str());
        } else if (propertyId == UIA_ControlTypePropertyId) {
            pRetVal->vt = VT_I4;
            switch (target->role) {
                case AccessibilityRole::Button: pRetVal->lVal = UIA_ButtonControlTypeId; break;
                case AccessibilityRole::Slider: pRetVal->lVal = UIA_SliderControlTypeId; break;
                case AccessibilityRole::TextInput: pRetVal->lVal = UIA_EditControlTypeId; break;
                default: pRetVal->lVal = UIA_GroupControlTypeId; break;
            }
        } else if (propertyId == UIA_BoundingRectanglePropertyId) {
            pRetVal->vt = VT_ARRAY | VT_R8;
            SAFEARRAY* psa = SafeArrayCreateVector(VT_R8, 0, 4);
            double bounds[] = { target->x, target->y, target->w, target->h };
            for (long i = 0; i < 4; i++) SafeArrayPutElement(psa, &i, &bounds[i]);
            pRetVal->parray = psa;
        }
        return S_OK;
    }

    IFACEMETHODIMP get_HostRawElementProvider(IRawElementProviderSimple** pRetVal) {
        if (m_hwnd == nullptr) return E_INVALIDARG;
        return UiaHostProviderFromHwnd(m_hwnd, pRetVal);
    }

private:
    long m_ref_count;
    ID m_id;
    HWND m_hwnd;
};

// Root Provider for the Window
class FantaRootProvider : public IRawElementProviderSimple, public IRawElementProviderFragmentRoot {
public:
    FantaRootProvider(HWND hwnd) : m_ref_count(1), m_hwnd(hwnd) {}

    // IUnknown
    IFACEMETHODIMP QueryInterface(REFIID riid, void** ppv) {
        if (riid == IID_IUnknown || riid == IID_IRawElementProviderSimple) *ppv = static_cast<IRawElementProviderSimple*>(this);
        else if (riid == IID_IRawElementProviderFragmentRoot) *ppv = static_cast<IRawElementProviderFragmentRoot*>(this);
        else { *ppv = nullptr; return E_NOINTERFACE; }
        AddRef(); return S_OK;
    }
    IFACEMETHODIMP_(ULONG) AddRef() { return InterlockedIncrement(&m_ref_count); }
    IFACEMETHODIMP_(ULONG) Release() { long res = InterlockedDecrement(&m_ref_count); if (res == 0) delete this; return res; }

    // IRawElementProviderSimple
    IFACEMETHODIMP get_ProviderOptions(ProviderOptions* pRetVal) { *pRetVal = ProviderOptions_ServerSideProvider; return S_OK; }
    IFACEMETHODIMP GetPatternProvider(PATTERNID patternId, IUnknown** pRetVal) { *pRetVal = nullptr; return S_OK; }
    IFACEMETHODIMP GetPropertyValue(PROPERTYID propertyId, VARIANT* pRetVal) {
        pRetVal->vt = VT_EMPTY;
        if (propertyId == UIA_NamePropertyId) { pRetVal->vt = VT_BSTR; pRetVal->bstrVal = SysAllocString(L"Fantasmagorie App"); }
        return S_OK;
    }
    IFACEMETHODIMP get_HostRawElementProvider(IRawElementProviderSimple** pRetVal) { return UiaHostProviderFromHwnd(m_hwnd, pRetVal); }

    // IRawElementProviderFragmentRoot
    IFACEMETHODIMP ElementProviderFromPoint(double x, double y, IRawElementProviderFragment** pRetVal) { *pRetVal = nullptr; return S_OK; }
    IFACEMETHODIMP GetFocus(IRawElementProviderFragment** pRetVal) { *pRetVal = nullptr; return S_OK; }

private:
    long m_ref_count;
    HWND m_hwnd;
};

// Global Accessibility Bridge
class Win32AccessibilityBridge {
public:
    static Win32AccessibilityBridge& Get() {
        static Win32AccessibilityBridge instance;
        return instance;
    }

    LRESULT HandleUiaRequest(HWND hwnd, WPARAM wParam, LPARAM lParam) {
        if (lParam == UiaRootObjectId) {
            IRawElementProviderSimple* root = new FantaRootProvider(hwnd);
            LRESULT res = UiaReturnRawElementProvider(hwnd, wParam, lParam, root);
            root->Release();
            return res;
        }
        return 0;
    }
};

} // namespace internal
} // namespace fanta

// Exported C API for Backend to call
extern "C" LRESULT FantaHandleUiaRequest(HWND hwnd, WPARAM wParam, LPARAM lParam) {
    return fanta::internal::Win32AccessibilityBridge::Get().HandleUiaRequest(hwnd, wParam, lParam);
}
