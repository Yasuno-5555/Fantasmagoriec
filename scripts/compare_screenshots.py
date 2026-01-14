import sys
import os
import math

def print_error(msg):
    print(f"\033[91mError: {msg}\033[0m")

def print_success(msg):
    print(f"\033[92m{msg}\033[0m")

def install_pillow_msg():
    print("\n\033[93mPillow library is required for image comparison.\033[0m")
    print("Please install it using pip:")
    print("  pip install Pillow")
    print("\nOr if you are in a managed environment, please ask the administrator.")

try:
    from PIL import Image, ImageChops, ImageStat
except ImportError:
    print_error("Pillow (PIL) not found.")
    install_pillow_msg()
    sys.exit(1)

def compare_images(img1_path, img2_path, diff_output_path):
    if not os.path.exists(img1_path):
        print_error(f"File not found: {img1_path}")
        return False
    if not os.path.exists(img2_path):
        print_error(f"File not found: {img2_path}")
        return False

    try:
        img1 = Image.open(img1_path).convert('RGBA')
        img2 = Image.open(img2_path).convert('RGBA')
    except Exception as e:
        print_error(f"Failed to open images: {e}")
        return False

    if img1.size != img2.size:
        print_error(f"Image dimensions do not match: {img1.size} vs {img2.size}")
        return False

    print(f"Comparing images ({img1.size[0]}x{img1.size[1]})...")

    # Generate Diff Image (Absolute Difference)
    diff = ImageChops.difference(img1, img2)
    
    # Save Diff Image (Enhance visibility if needed, but raw diff is good for analysis)
    # To make it visible, we can invert it or boost levels, but let's keep raw difference.
    # Actually, adding an alpha background helps seeing transparency diffs.
    diff.save(diff_output_path)
    print_success(f"Difference image saved to: {diff_output_path}")

    # Calculate RMS Error
    stat = ImageStat.Stat(diff)
    # Sum of squares of differences for each channel
    sum_squares = sum(stat.sum2)
    num_pixels = img1.size[0] * img1.size[1] * 4 # RGBA
    rms = math.sqrt(sum_squares / num_pixels)

    print(f"RMS Error: {rms:.4f}")

    # Check for perfect match
    if rms == 0:
        print_success("Images are IDENTICAL.")
        return True
    else:
        print(f"\033[93mImages differ (RMS: {rms:.4f})\033[0m")
        # Analyze where the difference is coming from
        # e.g. purely shadows?
        return False

if __name__ == "__main__":
    if len(sys.argv) < 3:
        print("Usage: python compare_screenshots.py <image1> <image2> [diff_output]")
        sys.exit(1)

    img1 = sys.argv[1]
    img2 = sys.argv[2]
    out = "diff.png"
    if len(sys.argv) > 3:
        out = sys.argv[3]

    success = compare_images(img1, img2, out)
    sys.exit(0 if success else 1)
