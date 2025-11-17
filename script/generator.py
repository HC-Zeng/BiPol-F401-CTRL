from PIL import Image
import numpy as np


def png_to_lcd_fill_specific_color(png_path, output_c_path, target_rgb):
    """
    将PNG图片中指定RGB颜色的区域转换为STM32 LCD_Fill调用代码。
    只填充指定颜色区域，其他区域完全忽略。

    Args:
        png_path (str): 输入的PNG图片路径。
        output_c_path (str): 输出的C代码文件路径。
        target_rgb (tuple): 目标RGB颜色值，如 (136, 211, 26)
    """

    # 1. 打开图片并转换为RGB模式
    img = Image.open(png_path)
    img = img.convert('RGB')
    width, height = img.size
    pixels = img.load()

    print(f"图片尺寸: {width} x {height}")
    print(f"目标颜色: RGB{target_rgb}")

    # 2. 创建掩码：标记所有目标颜色像素
    # 使用容差来匹配相近颜色
    tolerance = 1  # 颜色匹配容差，可以根据需要调整

    mask = np.zeros((height, width), dtype=bool)
    for y in range(height):
        for x in range(width):
            r, g, b = pixels[x, y]
            # 检查颜色是否匹配（带容差）
            if (abs(r - target_rgb[0]) <= tolerance and
                    abs(g - target_rgb[1]) <= tolerance and
                    abs(b - target_rgb[2]) <= tolerance):
                mask[y, x] = True

    # 3. 查找并合并目标颜色的矩形区域
    visited = np.zeros((height, width), dtype=bool)
    rects = []  # 存储矩形信息 (sx, sy, ex, ey)

    for y in range(height):
        for x in range(width):
            # 如果这个像素不是目标颜色或已被处理，则跳过
            if not mask[y, x] or visited[y, x]:
                continue

            # 3.1 找到一个从 (x, y) 开始的最大矩形
            # 首先确定当前行的连续长度
            w = 1
            while (x + w < width and
                   mask[y, x + w] and
                   not visited[y, x + w]):
                w += 1

            # 然后向下扩展行，确保每一行在 [x, x+w) 范围内都是目标颜色且未被访问
            h = 1
            for h_inc in range(1, height - y):
                # 检查下一行从x到x+w的像素是否都满足条件
                row_ok = True
                for i in range(w):
                    if (y + h_inc >= height or
                            not mask[y + h_inc, x + i] or
                            visited[y + h_inc, x + i]):
                        row_ok = False
                        break
                if not row_ok:
                    break
                h += 1

            # 3.2 现在我们找到了一个矩形: 从 (x, y) 到 (x+w-1, y+h-1)
            rects.append((x, y, x + w - 1, y + h - 1))

            # 3.3 将这个矩形区域标记为已访问
            visited[y:y + h, x:x + w] = True

    print(f"找到 {len(rects)} 个目标颜色矩形区域")

    # 4. 生成C代码
    with open(output_c_path, 'w') as f:
        f.write("// Auto-generated from PNG to LCD_Fill code (Specific Color)\n")
        f.write("// Image: {}\n".format(png_path))
        f.write("// Target Color: RGB{}\n".format(target_rgb))
        f.write("// Size: {}x{}\n".format(width, height))
        f.write("// Total Rects: {}\n\n".format(len(rects)))

        f.write("void Draw_rounded_rectangle_edge(u16 bx, u16 by) {\n")
        for i, (sx, sy, ex, ey) in enumerate(rects):
            f.write(f"    LCD_Fill(bx+{sx}, by+{sy}, bx+{ex}, by+{ey}, LIME_GREEN); // Rect {i + 1}\n")
        f.write("}\n")

    print(f"C代码已生成至: {output_c_path}")

    # 显示统计信息
    target_pixel_count = np.sum(mask)
    filled_pixel_count = sum((ex - sx + 1) * (ey - sy + 1) for sx, sy, ex, ey in rects)
    print(f"目标颜色像素数: {target_pixel_count}")
    print(f"填充矩形覆盖像素数: {filled_pixel_count}")

    if target_pixel_count > 0:
        efficiency = filled_pixel_count / target_pixel_count
        print(f"优化效率: {efficiency:.1%} (覆盖/总数)")

    # 检查是否有未覆盖的像素
    uncovered = target_pixel_count - filled_pixel_count
    if uncovered > 0:
        print(f"警告: 有 {uncovered} 个像素未被矩形覆盖")
    else:
        print("完美: 所有目标像素都被矩形覆盖")


# 使用示例
if __name__ == "__main__":
    # 参数配置
    input_png = "rounded_rectangle_edge.png"  # 您的PNG图片路径
    output_c = "rounded_rectangle_edge_code.c"  # 生成的C文件名

    # 指定目标颜色 (RGB 136, 211, 26)
    # TARGET_COLOR = (136, 211, 26)

    TARGET_COLOR = (204, 188, 188)

    # 执行转换
    png_to_lcd_fill_specific_color(input_png, output_c, TARGET_COLOR)