import tkinter as tk

# Grid size
COLUMNS = 64
ROWS = 32
PIXEL_SIZE = 15

COLOR_OFF = 0
COLOR_A = 1  # black
COLOR_B = 2  # red

class PixelEditor:
    def __init__(self, root):
        self.root = root
        self.root.title("64x32 Pixel Editor (2 Colors)")

        self.canvas = tk.Canvas(
            root,
            width=COLUMNS * PIXEL_SIZE,
            height=ROWS * PIXEL_SIZE,
            bg="white"
        )
        self.canvas.pack()

        self.export_button = tk.Button(
            root,
            text="Export C Arrays",
            command=self.export_c_arrays
        )
        self.export_button.pack(pady=5)

        # Pixel data
        self.pixels = [[COLOR_OFF for _ in range(COLUMNS)] for _ in range(ROWS)]
        self.rects = [[None for _ in range(COLUMNS)] for _ in range(ROWS)]

        self.draw_grid()

        # Mouse bindings
        self.canvas.bind("<Button-1>", self.paint_color_a)
        self.canvas.bind("<B1-Motion>", self.paint_color_a)

        self.canvas.bind("<Button-3>", self.paint_color_b)
        self.canvas.bind("<B3-Motion>", self.paint_color_b)

    def draw_grid(self):
        for y in range(ROWS):
            for x in range(COLUMNS):
                x1 = x * PIXEL_SIZE
                y1 = y * PIXEL_SIZE
                x2 = x1 + PIXEL_SIZE
                y2 = y1 + PIXEL_SIZE

                rect = self.canvas.create_rectangle(
                    x1, y1, x2, y2,
                    fill="white",
                    outline="gray"
                )
                self.rects[y][x] = rect

    def update_pixel(self, x, y):
        value = self.pixels[y][x]
        if value == COLOR_A:
            color = "black"
        elif value == COLOR_B:
            color = "red"
        else:
            color = "white"

        self.canvas.itemconfig(self.rects[y][x], fill=color)

    def paint_color_a(self, event):
        self.paint(event, COLOR_A)

    def paint_color_b(self, event):
        self.paint(event, COLOR_B)

    def paint(self, event, color):
        x = event.x // PIXEL_SIZE
        y = event.y // PIXEL_SIZE

        if 0 <= x < COLUMNS and 0 <= y < ROWS:
            # paint behavior (no toggle flicker)
            self.pixels[y][x] = color
            self.update_pixel(x, y)

    def export_c_arrays(self):
        print("\n// Color A (black)")
        print("static const uint8_t arrayA[ROWS][COLUMNS] = {")
        for row in self.pixels:
            print("    {" + ",".join("1" if v == COLOR_A else "0" for v in row) + "},")
        print("};\n")

        print("// Color B (red)")
        print("static const uint8_t arrayB[ROWS][COLUMNS] = {")
        for row in self.pixels:
            print("    {" + ",".join("1" if v == COLOR_B else "0" for v in row) + "},")
        print("};\n")

if __name__ == "__main__":
    root = tk.Tk()
    PixelEditor(root)
    root.mainloop()
