import fanta
import time

# Persistent state in Python (survives logic reloads if we handle it right, 
# but for now, it's global to the module)
click_count = 0
start_time = time.time()

def render():
    global click_count
    
    w = float(fanta.get_width())
    h = float(fanta.get_height())
    elapsed = time.time() - start_time
    
    # --- UI Definition (Fluent API) ---
    root = fanta.Column().width(w).height(h).padding(20).bg(fanta.Color(0.06, 0.06, 0.08, 1.0))
    
    # Header
    fanta.Row().height(70).padding(15).margin(10).bg(fanta.Color(0.18, 0.2, 0.28, 1.0)).radius(15).shadow(20)
    fanta.Text("Ouroboros: Live Hot Reload").size(24).color(fanta.Color(0.95, 0.95, 1.0, 1.0))
    fanta.End()
    
    fanta.Scroll().grow(1).margin(5).padding(15).bg(fanta.Color(0.07, 0.07, 0.09, 1.0)).radius(12)
    
    for i in range(10):
        fanta.Box().height(80).margin(8).bg(fanta.Color(0.12, 0.12, 0.16, 1.0)).radius(12).shadow(10)
        fanta.Row().grow(1).padding(12).align(fanta.Align.Center)
        
        fanta.Text(f"Hot Reload Item {i+1}").size(16).grow(1)
        
        if fanta.Button(f"Action {i}").width(110).height(45).radius(10).bg(fanta.Color(0.3, 0.4, 0.7)):
            click_count += 1
            print(f"PyLog: Action {i} triggered! Total: {click_count}")
            
        fanta.End()
        fanta.End()
        
    fanta.End() # Scroll
    
    # Footer
    fanta.Row().height(40).padding(10).bg(fanta.Color(0.04, 0.04, 0.05, 1.0))
    fanta.Text(f"Clicks: {click_count} | Time: {elapsed:.1f}s").size(14).color(fanta.Color(0.5, 0.8, 0.5))
    fanta.End()
    
    fanta.End() # Root
    
    return root, w, h
