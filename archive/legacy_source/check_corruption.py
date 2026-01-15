import os

def check_files(start_dir):
    corrupted = []
    for root, dirs, files in os.walk(start_dir):
        for f in files:
            if f.endswith('.cpp') or f.endswith('.hpp'):
                path = os.path.join(root, f)
                try:
                    with open(path, 'rb') as fin:
                        content = fin.read()
                        content.decode('utf-8')
                except Exception as e:
                    corrupted.append((path, str(e)))
    return corrupted

if __name__ == "__main__":
    results = check_files('src')
    if not results:
        print("No corrupted files found.")
    else:
        print(f"Found {len(results)} corrupted files:")
        for p, err in results:
            print(f"  {p}: {err}")
