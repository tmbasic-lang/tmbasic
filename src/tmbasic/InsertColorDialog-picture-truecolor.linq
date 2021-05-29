<Query Kind="Statements">
  <Reference Relative="ColorHelper.dll">C:\Users\Brian Luft\Desktop\ColorHelper.dll</Reference>
</Query>

// needs https://www.nuget.org/packages/ColorHelper/

System.Drawing.Color makeColor(int r, int g, int b) => System.Drawing.Color.FromArgb(r, g, b);

var colors16 = new System.Drawing.Color[] {
	makeColor(0, 0, 0), makeColor(128, 128, 128), makeColor(128, 0, 0), makeColor(128, 128, 0),
	makeColor(0, 128, 0), makeColor(0, 128, 128), makeColor(0, 0, 128), makeColor(128, 0, 128),
	makeColor(255, 255, 255), makeColor(192, 192, 192), makeColor(255, 0, 0), makeColor(255, 255, 0),
	makeColor(0, 255, 0), makeColor(0, 255, 255), makeColor(0, 0, 255), makeColor(255, 0, 255),      
};

using (var bitmap = new System.Drawing.Bitmap(620, 400))
using (var graphics = System.Drawing.Graphics.FromImage(bitmap)) {
	void DrawChart(byte saturation, int rows, int top) {
		for (var row = 0; row < rows; row++) {
			var y = top + row * 20;
			var brightness = 5 + 80 * (1 - (double)row / rows);
			for (var x = 0; x < bitmap.Width; x++) {
				var hue = 360 * (double)x / bitmap.Width;
				var rgb = ColorHelper.ColorConverter.HslToRgb(new ColorHelper.HSL((int)hue, (byte)saturation, (byte)brightness));
				var color = makeColor(rgb.R, rgb.G, rgb.B);
				using (var brush = new System.Drawing.SolidBrush(color)) {
					graphics.FillRectangle(brush, x, y, 1, 20);
				}
			}
		}
	}
	
	void DrawGrays(int top) {
		for (var x = 0; x < bitmap.Width; x++) {
			var grayValue = 255 - (int)(((double)x / bitmap.Width) * 255);
			var color = makeColor(grayValue, grayValue, grayValue);
			using (var brush = new System.Drawing.SolidBrush(color)) {
				graphics.FillRectangle(brush, x, top, 1, 20);
			}
		}
	}
	
	void Draw16(int top) {
		var w = bitmap.Width;
		var colorW = (double)w / 8;
		double x = 0;
		foreach (var color in colors16.Skip(8).Take(8)) {
			using (var brush = new System.Drawing.SolidBrush(color)) {
				graphics.FillRectangle(brush, (int)x, top, (int)(colorW + 1), 20);
				x += colorW;
			}
		}
		x = 0;
		top += 20;
		foreach (var color in colors16.Take(8)) {
			using (var brush = new System.Drawing.SolidBrush(color)) {
				graphics.FillRectangle(brush, (int)x, top, (int)(colorW + 1), 20);
				x += colorW;
			}
		}
	}
	
	DrawGrays(340);
	Draw16(0);
	DrawChart(100, 10, 40);
	DrawChart(35, 5, 240);
	
	var sb = new StringBuilder();
	
	string rgb(int red, int green, int blue) {
		return "0x" +
		red.ToString("x").PadLeft(2,'0') +
		green.ToString("x").PadLeft(2,'0') +
		blue.ToString("x").PadLeft(2,'0');
	}
	
	var distinctColors = new HashSet<string>();
	using (var brush = new System.Drawing.SolidBrush(System.Drawing.Color.White)) {
		for (var yi = 0; yi < 18; yi++) {
			var y = 10 + yi*20;
			for (var xi = 0; xi < 62; xi++) {
				var x = 5 + (int)(xi*10);
				
				var color = bitmap.GetPixel(x,y);
				var hex = rgb(color.R, color.G, color.B);
				distinctColors.Add(hex);
				sb.Append(hex);
				sb.Append(", ");
				
				graphics.FillRectangle(brush, x, y, 1, 1);
			}
			sb.Append("\n");
		}
	}
	
	bitmap.Dump();
	distinctColors.Count.Dump();
	sb.ToString().Dump();
}
