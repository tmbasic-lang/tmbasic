<Query Kind="Statements" />

System.Drawing.Color makeColor(int r, int g, int b) => System.Drawing.Color.FromArgb(r, g, b);

var colors16 = new System.Drawing.Color[] {
	makeColor(0, 0, 0), makeColor(128, 128, 128), makeColor(128, 0, 0), makeColor(128, 128, 0),
	makeColor(0, 128, 0), makeColor(0, 128, 128), makeColor(0, 0, 128), makeColor(128, 0, 128),
	makeColor(255, 255, 255), makeColor(192, 192, 192), makeColor(255, 0, 0), makeColor(255, 255, 0),
	makeColor(0, 255, 0), makeColor(0, 255, 255), makeColor(0, 0, 255), makeColor(255, 0, 255),      
};

var colors = new System.Drawing.Color[] {
    makeColor(0, 0, 0),       makeColor(128, 0, 0),     makeColor(0, 128, 0),     makeColor(128, 128, 0),
    makeColor(0, 0, 128),     makeColor(128, 0, 128),   makeColor(0, 128, 128),   makeColor(192, 192, 192),
    makeColor(128, 128, 128), makeColor(255, 0, 0),     makeColor(0, 255, 0),     makeColor(255, 255, 0),
    makeColor(0, 0, 255),     makeColor(255, 0, 255),   makeColor(0, 255, 255),   makeColor(255, 255, 255),
    makeColor(0, 0, 0),       makeColor(0, 0, 95),      makeColor(0, 0, 135),     makeColor(0, 0, 175),
    makeColor(0, 0, 215),     makeColor(0, 0, 255),     makeColor(0, 95, 0),      makeColor(0, 95, 95),
    makeColor(0, 95, 135),    makeColor(0, 95, 175),    makeColor(0, 95, 215),    makeColor(0, 95, 255),
    makeColor(0, 135, 0),     makeColor(0, 135, 95),    makeColor(0, 135, 135),   makeColor(0, 135, 175),
    makeColor(0, 135, 215),   makeColor(0, 135, 255),   makeColor(0, 175, 0),     makeColor(0, 175, 95),
    makeColor(0, 175, 135),   makeColor(0, 175, 175),   makeColor(0, 175, 215),   makeColor(0, 175, 255),
    makeColor(0, 215, 0),     makeColor(0, 215, 95),    makeColor(0, 215, 135),   makeColor(0, 215, 175),
    makeColor(0, 215, 215),   makeColor(0, 215, 255),   makeColor(0, 255, 0),     makeColor(0, 255, 95),
    makeColor(0, 255, 135),   makeColor(0, 255, 175),   makeColor(0, 255, 215),   makeColor(0, 255, 255),
    makeColor(95, 0, 0),      makeColor(95, 0, 95),     makeColor(95, 0, 135),    makeColor(95, 0, 175),
    makeColor(95, 0, 215),    makeColor(95, 0, 255),    makeColor(95, 95, 0),     makeColor(95, 95, 95),
    makeColor(95, 95, 135),   makeColor(95, 95, 175),   makeColor(95, 95, 215),   makeColor(95, 95, 255),
    makeColor(95, 135, 0),    makeColor(95, 135, 95),   makeColor(95, 135, 135),  makeColor(95, 135, 175),
    makeColor(95, 135, 215),  makeColor(95, 135, 255),  makeColor(95, 175, 0),    makeColor(95, 175, 95),
    makeColor(95, 175, 135),  makeColor(95, 175, 175),  makeColor(95, 175, 215),  makeColor(95, 175, 255),
    makeColor(95, 215, 0),    makeColor(95, 215, 95),   makeColor(95, 215, 135),  makeColor(95, 215, 175),
    makeColor(95, 215, 215),  makeColor(95, 215, 255),  makeColor(95, 255, 0),    makeColor(95, 255, 95),
    makeColor(95, 255, 135),  makeColor(95, 255, 175),  makeColor(95, 255, 215),  makeColor(95, 255, 255),
    makeColor(135, 0, 0),     makeColor(135, 0, 95),    makeColor(135, 0, 135),   makeColor(135, 0, 175),
    makeColor(135, 0, 215),   makeColor(135, 0, 255),   makeColor(135, 95, 0),    makeColor(135, 95, 95),
    makeColor(135, 95, 135),  makeColor(135, 95, 175),  makeColor(135, 95, 215),  makeColor(135, 95, 255),
    makeColor(135, 135, 0),   makeColor(135, 135, 95),  makeColor(135, 135, 135), makeColor(135, 135, 175),
    makeColor(135, 135, 215), makeColor(135, 135, 255), makeColor(135, 175, 0),   makeColor(135, 175, 95),
    makeColor(135, 175, 135), makeColor(135, 175, 175), makeColor(135, 175, 215), makeColor(135, 175, 255),
    makeColor(135, 215, 0),   makeColor(135, 215, 95),  makeColor(135, 215, 135), makeColor(135, 215, 175),
    makeColor(135, 215, 215), makeColor(135, 215, 255), makeColor(135, 255, 0),   makeColor(135, 255, 95),
    makeColor(135, 255, 135), makeColor(135, 255, 175), makeColor(135, 255, 215), makeColor(135, 255, 255),
    makeColor(175, 0, 0),     makeColor(175, 0, 95),    makeColor(175, 0, 135),   makeColor(175, 0, 175),
    makeColor(175, 0, 215),   makeColor(175, 0, 255),   makeColor(175, 95, 0),    makeColor(175, 95, 95),
    makeColor(175, 95, 135),  makeColor(175, 95, 175),  makeColor(175, 95, 215),  makeColor(175, 95, 255),
    makeColor(175, 135, 0),   makeColor(175, 135, 95),  makeColor(175, 135, 135), makeColor(175, 135, 175),
    makeColor(175, 135, 215), makeColor(175, 135, 255), makeColor(175, 175, 0),   makeColor(175, 175, 95),
    makeColor(175, 175, 135), makeColor(175, 175, 175), makeColor(175, 175, 215), makeColor(175, 175, 255),
    makeColor(175, 215, 0),   makeColor(175, 215, 95),  makeColor(175, 215, 135), makeColor(175, 215, 175),
    makeColor(175, 215, 215), makeColor(175, 215, 255), makeColor(175, 255, 0),   makeColor(175, 255, 95),
    makeColor(175, 255, 135), makeColor(175, 255, 175), makeColor(175, 255, 215), makeColor(175, 255, 255),
    makeColor(215, 0, 0),     makeColor(215, 0, 95),    makeColor(215, 0, 135),   makeColor(215, 0, 175),
    makeColor(215, 0, 215),   makeColor(215, 0, 255),   makeColor(215, 95, 0),    makeColor(215, 95, 95),
    makeColor(215, 95, 135),  makeColor(215, 95, 175),  makeColor(215, 95, 215),  makeColor(215, 95, 255),
    makeColor(215, 135, 0),   makeColor(215, 135, 95),  makeColor(215, 135, 135), makeColor(215, 135, 175),
    makeColor(215, 135, 215), makeColor(215, 135, 255), makeColor(215, 175, 0),   makeColor(215, 175, 95),
    makeColor(215, 175, 135), makeColor(215, 175, 175), makeColor(215, 175, 215), makeColor(215, 175, 255),
    makeColor(215, 215, 0),   makeColor(215, 215, 95),  makeColor(215, 215, 135), makeColor(215, 215, 175),
    makeColor(215, 215, 215), makeColor(215, 215, 255), makeColor(215, 255, 0),   makeColor(215, 255, 95),
    makeColor(215, 255, 135), makeColor(215, 255, 175), makeColor(215, 255, 215), makeColor(215, 255, 255),
    makeColor(255, 0, 0),     makeColor(255, 0, 95),    makeColor(255, 0, 135),   makeColor(255, 0, 175),
    makeColor(255, 0, 215),   makeColor(255, 0, 255),   makeColor(255, 95, 0),    makeColor(255, 95, 95),
    makeColor(255, 95, 135),  makeColor(255, 95, 175),  makeColor(255, 95, 215),  makeColor(255, 95, 255),
    makeColor(255, 135, 0),   makeColor(255, 135, 95),  makeColor(255, 135, 135), makeColor(255, 135, 175),
    makeColor(255, 135, 215), makeColor(255, 135, 255), makeColor(255, 175, 0),   makeColor(255, 175, 95),
    makeColor(255, 175, 135), makeColor(255, 175, 175), makeColor(255, 175, 215), makeColor(255, 175, 255),
    makeColor(255, 215, 0),   makeColor(255, 215, 95),  makeColor(255, 215, 135), makeColor(255, 215, 175),
    makeColor(255, 215, 215), makeColor(255, 215, 255), makeColor(255, 255, 0),   makeColor(255, 255, 95),
    makeColor(255, 255, 135), makeColor(255, 255, 175), makeColor(255, 255, 215), makeColor(255, 255, 255),
    makeColor(8, 8, 8),       makeColor(18, 18, 18),    makeColor(28, 28, 28),    makeColor(38, 38, 38),
    makeColor(48, 48, 48),    makeColor(58, 58, 58),    makeColor(68, 68, 68),    makeColor(78, 78, 78),
    makeColor(88, 88, 88),    makeColor(98, 98, 98),    makeColor(108, 108, 108), makeColor(118, 118, 118),
    makeColor(128, 128, 128), makeColor(138, 138, 138), makeColor(148, 148, 148), makeColor(158, 158, 158),
    makeColor(168, 168, 168), makeColor(178, 178, 178), makeColor(188, 188, 188), makeColor(198, 198, 198),
    makeColor(208, 208, 208), makeColor(218, 218, 218), makeColor(228, 228, 228), makeColor(238, 238, 238),

};

colors.Length.Dump();
colors = colors.Distinct().ToArray();
colors.Length.Dump();

using (var bitmap = new System.Drawing.Bitmap(620, 400))
using (var graphics = System.Drawing.Graphics.FromImage(bitmap)) {
	void DrawChart(Func<float, bool> saturationFilter, int top, bool flip) {
		var brightnesses = colors.Where(x => saturationFilter(x.GetSaturation()))
			.Select(x => x.GetBrightness()).Distinct().OrderBy(x => x).ToList();
		var w = bitmap.Width;
		for (var bIndex = 0; bIndex < brightnesses.Count; bIndex++) {
			var y = bIndex * 20;
			var bColors = colors
				.Where(x => x.GetBrightness() == brightnesses[bIndex] && saturationFilter(x.GetSaturation()))
				.OrderBy(x => x.GetHue() + x.GetSaturation())
				.ToList();
			for (var colorIndex = 0; colorIndex < bColors.Count; colorIndex++) {
				var x = colorIndex * w / bColors.Count;
				
				using (var brush = new System.Drawing.SolidBrush(bColors[colorIndex])) {
					graphics.FillRectangle(brush, x, flip ? top - y : y + top, w / bColors.Count + 1, 20);
				}
			}
		}
	}
	
	void DrawGrays(int top) {
		var grays = colors.Where(x => x.GetSaturation() == 0).Distinct().OrderBy(x => x.GetBrightness()).ToList();
		var w = bitmap.Width;
		var colorW = (double)w / grays.Count;
		double x = 0;
		grays.Reverse();
		foreach (var gray in grays) {
			using (var brush = new System.Drawing.SolidBrush(gray)) {
				graphics.FillRectangle(brush, (int)x, top, (int)(colorW + 1), 20);
				x += colorW;
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
	
	DrawChart(x => x == 1, 220, true);
	DrawChart(x => x != 0 && x != 1, 320, true);
	DrawGrays(340);
	Draw16(0);
	
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
