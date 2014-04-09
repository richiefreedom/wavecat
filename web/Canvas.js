function drawPixel(ctx, x, y)
{
	ctx.beginPath();
	ctx.rect(x, y, 1, 1);
	ctx.fill();
	ctx.closePath();
}

function drawPixelScaled(ctx, scale, x, y)
{
	drawPixel(ctx, x * scale, y * scale);
}

function drawLine(ctx, x0, y0, x1, y1)
{
	ctx.beginPath();
	ctx.moveTo(x0, y0);
	ctx.lineTo(x1, y1);
	ctx.closePath();
	ctx.stroke();
}

function drawLineScaled(ctx, scale, x0, y0, x1, y1)
{
	drawLine(ctx, x0 * scale, y0 * scale, x1 * scale, y1 * scale);
}

function drawLineScaledInv(ctx, scale, x0, y0, x1, y1)
{
	drawLineScaled(ctx, scale, y0, x0, y1, x1);
}

function drawTextScaled(ctx, scale, text, x, y)
{
		ctx.fillText(text, x * scale, y * scale);
}

function canvasTranslateScaled(ctx, scale, x, y)
{
	ctx.translate(x * scale, y * scale);
}

function moveToScaled(ctx, scale, x, y)
{
	ctx.moveTo(scale * x, scale * y);
}

function lineToScaled(ctx, scale, x, y)
{
	ctx.lineTo(scale * x, scale * y);
}

function moveToScaledInv(ctx, scale, x, y)
{
	moveToScaled(ctx, scale, y, x);
}

function lineToScaledInv(ctx, scale, x, y)
{
	lineToScaled(ctx, scale, y, x);	
}
