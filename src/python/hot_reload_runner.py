import fanta
import importlib
import time
import os
import sys
import traceback

# Windows DLL Search Path Fix (Required for MinGW builds)
if sys.platform == "win32":
    mingw_bin = "C:\\mingw64\\bin"
    if os.path.exists(mingw_bin):
        os.add_dll_directory(mingw_bin)
    # Also ensure current build dir is reachable
    os.add_dll_directory(os.getcwd())

# Ensure we can import from src/python
sys.path.append(os.path.join(os.getcwd(), "../src/python"))

def main():
    print("Project Ouroboros: Initializing Serpent Runner... 🐍🔄")
    
    # 1. Initialize Engine (Once)
    fanta.init(1280, 800, "Fantasmagorie V5: Project Ouroboros 🔄")
    
    # 2. Dynamic Import of UI
    import test_ui as ui_module
    
    last_mtime = 0
    try:
        last_mtime = os.path.getmtime(ui_module.__file__)
    except:
        pass

    print("Running... Edit src/python/test_ui.py to see live changes!")
    
    while fanta.running():
        # A. Start Engine Frame
        fanta.begin_frame()

        # B. Hot Reload Logic
        try:
            current_mtime = os.path.getmtime(ui_module.__file__)
            if current_mtime > last_mtime:
                print("🔄 Source changed! Reloading UI module...")
                importlib.reload(ui_module)
                last_mtime = current_mtime
        except Exception as e:
            # File might be locked during save, skip a frame
            pass

        # C. Render UI Tree
        try:
            # The test_ui.render() returns (root, width, height)
            root, w, h = ui_module.render()
            fanta.render_ui(root, w, h)
        except Exception as e:
            # Failure in Python code shouldn't crash the engine!
            # We construct a simple emergency UI in C++ to show the error.
            err_msg = traceback.format_exc()
            
            w = float(fanta.get_width())
            h = float(fanta.get_height())
            
            root = fanta.Column().width(w).height(h).padding(50).bg(fanta.Color(0.2, 0, 0, 1.0))
            fanta.Text("🐍 PYTHON CRASH").size(40).color(fanta.Color(1, 0.2, 0.2, 1.0))
            fanta.Box().height(20) # Spacer
            fanta.Text(err_msg).size(14).color(fanta.Color(0.9, 0.9, 0.9, 1.0)).grow(1)
            fanta.End()
            
            fanta.render_ui(root, w, h)

    print("Ouroboros cycle complete.")

if __name__ == "__main__":
    main()
