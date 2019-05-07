using System;
using System.Drawing;

namespace ConsoleApp1
{
    class Program
    {
        static void Main(string[] args)
        {
            var bitmap = new Bitmap(Image.FromFile(args[0]));
            var sb = new System.Text.StringBuilder(
                "const uint16_t picture[" + bitmap.Height.ToString()
                + "][" + bitmap.Width.ToString() + "] = { " + Environment.NewLine);
            for (int i = 0; i < bitmap.Height; ++i)
            {
                sb.Append("{ ");
                for (int j = 0; j < bitmap.Width; ++j)
                {
                    uint argb = (uint)bitmap.GetPixel(j, bitmap.Height - 1 - i).ToArgb();
                    uint r = (argb >> (16 + 3)) & 0x1F;
                    uint g = (argb >> (8 + 2)) & 0x3F;
                    uint b = (argb >> (0 + 3)) & 0x1F;
                    ushort rgb = (ushort)((r << (6 + 5)) | (g << 5) | b);
                    sb.Append("0x" + rgb.ToString("X4"));
                    sb.Append((j == bitmap.Width - 1) ? "" : ", ");
                }
                sb.Append(" }" + ((i == bitmap.Height - 1) ? "" : ",") + Environment.NewLine);
            }
            sb.Append(" };");
            var sw = new System.IO.StreamWriter(System.IO.Path.ChangeExtension(args[0], "txt"));
            sw.Write(sb.ToString());
            sw.Close();
        }
    }
}
