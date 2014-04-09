function drawAxesScaled(ctx, scale, x0, y0, experimentData)
{
	x1 = experimentData.data[0].length;
	y1 = experimentData.data.length;
	startX = experimentData.minX;
	startY = experimentData.minY;
	endX = experimentData.maxX;
	endY = experimentData.maxY;

	drawLineScaled(ctx, scale, x0, y0, x1, y0);
	drawLineScaled(ctx, scale, x1, y0, x1, y1);
	drawLineScaled(ctx, scale, x1, y1, x0, y1);
	drawLineScaled(ctx, scale, x0, y1, x0, y0);

	var width = x1 - x0;
	var height = y1 - y0;
	var dX = (endX - startX)/width;
	var dY = (endY - startY)/height;

	var textShift = 10;
	var needPrint = 0;

	for (i = x0; i <= width; i+=10) {
		needPrint = (needPrint)? 0:1;
		drawLineScaled(ctx, scale, i, y0, i, y0+1);
		drawLineScaled(ctx, scale, i, y0+height, i, y0+height-1);
		if (needPrint)
			drawTextScaled(ctx, scale,
				(startX + i*dX).toFixed(1).toString(), i-5,
				y0 + height + textShift);
		for (j = y0; j <= height; j+=10) {
			if (i == x0) {
				drawLineScaled(ctx, scale, x0, j, x0+1, j);
				drawLineScaled(ctx, scale, x0+width, j,
					x0+width-1, j);
				drawTextScaled(ctx, scale,
					(startY + j*dY).toFixed(1).toString(),
					x0+width+10, j+2);
			}
			drawPixelScaled(ctx, scale, i, j);
		}
	}
}
