import os
import shutil

def deep_audit_and_destroy(start_dir):
    ghosts_found = []
    
    # We use list(os.walk) to avoid issues if we delete directories we are currently walking
    for root, dirs, files in list(os.walk(start_dir, topdown=False)):
        # 1. Look for directories that should be files
        for d in dirs:
            if d.endswith('.hpp') or d.endswith('.cpp') or d.endswith('.h') or '..' in d:
                path = os.path.join(root, d)
                print(f"CRITICAL: Found ghost directory: {path}")
                try:
                    shutil.rmtree(path)
                    ghosts_found.append(path)
                except Exception as e:
                    print(f"Failed to delete {path}: {e}")
        
        # 2. Look for malformed files
        for f in files:
            is_malformed = (
                len(f.split('.')) > 2 or 
                '..' in f or 
                'hppppp' in f or 
                'Resolv' in f or
                'stj' in f or
                'exj' in f
            )
            if is_malformed:
                path = os.path.join(root, f)
                print(f"Found glitched file: {path}")
                try:
                    os.remove(path)
                    ghosts_found.append(path)
                except Exception as e:
                    print(f"Failed to delete {path}: {e}")

    return ghosts_found

if __name__ == "__main__":
    print("Starting Deep Atomic Cleanup...")
    ghosts = deep_audit_and_destroy('src')
    if not ghosts:
        print("Filesystem appears healthy (no ghosts found).")
    else:
        print(f"Cleanup complete. Removed {len(ghosts)} items.")
