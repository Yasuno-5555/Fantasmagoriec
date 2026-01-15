import os

def aggressive_cleanup(start_dir):
    # Patterns that indicate corruption or ghost files
    ghost_patterns = [
        '.hppppp', 
        'hpp.hpp', 
        'cpp.cpp', 
        '..', 
        'exj', 
        'stj', 
        'Resolv',
        'cE:', # Error from previous glitched writes
        '.hpp.',
        '.cpp.'
    ]
    
    deleted = []
    for root, dirs, files in os.walk(start_dir):
        for f in files:
            path = os.path.join(root, f)
            
            # Condition 1: Filename has more than one extension dot (except .tar.gz etc, but we don't have those)
            # e.g., node.hpp.hpp, types.hppppp.hpp
            if len(f.split('.')) > 2:
                try:
                    os.remove(path)
                    deleted.append(path)
                    continue
                except: pass

            # Condition 2: Filename matches known corruption patterns
            if any(p in f for p in ghost_patterns):
                try:
                    os.remove(path)
                    deleted.append(path)
                    continue
                except: pass
                
            # Condition 3: File is empty or too short (heuristic for corrupted headers)
            # but we'll be careful here. Only if it's explicitly a header that should be longer.
            if f in ['node.hpp', 'types.hpp', 'widget_base.hpp'] and os.path.getsize(path) < 100:
                 # Don't delete yet, just log
                 print(f"Warning: {path} is very small ({os.path.getsize(path)} bytes)")

    return deleted

if __name__ == "__main__":
    print("Starting aggressive cleanup of 'src' directory...")
    deleted_files = aggressive_cleanup('src')
    if not deleted_files:
        print("No ghost files found.")
    else:
        print(f"Successfully deleted {len(deleted_files)} ghost/corrupted files:")
        for p in deleted_files:
            print(f"  - {p}")
