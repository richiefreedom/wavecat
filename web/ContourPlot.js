/*  p0    p1
 *   +---+
 *   |   |
 *   +---+
 *  p3    p2
 */
function classifyCell(p0, p1, p2, p3, threshold)
{
	var p0Mark = (p0 > threshold) ? 1 : 0;
	var p1Mark = (p1 > threshold) ? 1 : 0;
	var p2Mark = (p2 > threshold) ? 1 : 0;
	var p3Mark = (p3 > threshold) ? 1 : 0;

	return (p0Mark << 3) | (p1Mark << 2) | (p2Mark << 1) | p3Mark;
}

function interpRatio(p0, p1, threshold)
{
	var epsilon = 0.0000001;
	var ratio;

	if (Math.abs(threshold - p0) < epsilon) return 0.0;
	if (Math.abs(threshold - p1) < epsilon) return 1.0;
	if (Math.abs(p0 - p1) < epsilon) return 1.0;

	ratio = (threshold - p0) / (p1 - p0);
	return ratio;
}

function drawThresholdPalette(ctx, scale, thresholds, colors)
{
	if (thresholds.length != colors.length) {
		alert("Error: disparity of threshold and color arrays");
		return;
	}

	for (t = 0; t < thresholds.length; t++) {
		ctx.strokeStyle = colors[t];
		drawLineScaled(ctx, scale, 0, t*5, 5, t*5);
		drawTextScaled(ctx, scale, thresholds[t].toFixed(2).toString(), 10, t*5);
	}
}

/*
 * Draw contour plot using the Marching Squares algorithm
 */
function drawContourPlot(ctx, scale, data, thresholds, colors)
{
	var ratio0;
	var ratio1;

	if (thresholds.length != colors.length) {
		alert("Error: disparity of threshold and color arrays");
		return;
	}

	for (t = 0; t < thresholds.length; t++) {
		ctx.strokeStyle = colors[t];

		for (i = 0; i < data.length - 1; i++) {
			for (j = 0 ; j < data[i].length - 1; j++) {
				var p0 = data[i][j];
				var p1 = data[i][j+1];
				var p2 = data[i+1][j+1];
				var p3 = data[i+1][j];

				var cellClass = classifyCell(p0, p1, p2, p3, thresholds[t]);
				switch (cellClass) {
					case 3: /* --- */
					case 12:
						ratio0 = interpRatio(data[i][j], data[i+1][j], thresholds[t]);
						ratio1 = interpRatio(data[i][j+1], data[i+1][j+1], thresholds[t]);
						drawLineScaledInv(ctx, scale, j, i+ratio0, j+1, i+ratio1);
						break;
					case 6: /*  |  */
					case 9:
						ratio0 = interpRatio(data[i][j], data[i][j+1], thresholds[t]);
						ratio1 = interpRatio(data[i+1][j], data[i+1][j+1], thresholds[t]);
						drawLineScaledInv(ctx, scale, j+ratio0, i, j+ratio1, i+1);
						break;
					case 1: /* .\  */
					case 14:
						ratio0 = interpRatio(data[i][j], data[i+1][j], thresholds[t]);
						ratio1 = interpRatio(data[i+1][j], data[i+1][j+1], thresholds[t]);
						drawLineScaledInv(ctx, scale, j, i+ratio0, j+ratio1, i+1);
						break;
					case 2: /*  /. */
					case 13:
						ratio0 = interpRatio(data[i+1][j], data[i+1][j+1], thresholds[t]);
						ratio1 = interpRatio(data[i][j+1], data[i+1][j+1], thresholds[t]);
						drawLineScaledInv(ctx, scale, j+ratio0, i+1, j+1, i+ratio1);
						break;
					case 4: /*  \` */
					case 11:
						ratio0 = interpRatio(data[i][j], data[i][j+1], thresholds[t]);
						ratio1 = interpRatio(data[i][j+1], data[i+1][j+1], thresholds[t]);
						drawLineScaledInv(ctx, scale, j+ratio0, i, j+1, i+ratio1);
						break;
					case 7: /* `/  */
					case 8:
						ratio0 = interpRatio(data[i][j], data[i+1][j], thresholds[t]);
						ratio1 = interpRatio(data[i][j], data[i][j+1], thresholds[t]);
						drawLineScaledInv(ctx, scale, j, i+ratio0, j+ratio1, i);
						break;
					case 5: /* / / */
						ratio0 = interpRatio(data[i][j], data[i+1][j], thresholds[t]);
						ratio1 = interpRatio(data[i][j], data[i][j+1], thresholds[t]);
						drawLineScaledInv(ctx, scale, j, i+ratio0, j+ratio1, i);

						ratio0 = interpRatio(data[i+1][j], data[i+1][j+1], thresholds[t]);
						ratio1 = interpRatio(data[i][j+1], data[i+1][j+1], thresholds[t]);
						drawLineScaledInv(ctx, scale, j+ratio0, i+1, j+1, i+ratio1);
						break;
					case 10: /* \ \ */
						ratio0 = interpRatio(data[i][j], data[i+1][j], thresholds[t]);
						ratio1 = interpRatio(data[i+1][j], data[i+1][j+1], thresholds[t]);
						drawLineScaledInv(ctx, scale, j, i+ratio0, j+ratio1, i+1);

						ratio0 = interpRatio(data[i][j], data[i][j+1], thresholds[t]);
						ratio1 = interpRatio(data[i][j+1], data[i+1][j+1], thresholds[t]);
						drawLineScaledInv(ctx, scale, j+ratio0, i, j+1, i+ratio1);
						break;
				}
			}
		}
	}
}

/*
 * Draw filled contour plot using the Marching Squares algorithm
 * This implementation is not so fast as simple stroke contour plot
 * function, because it should handle twice more cases.
 */
function drawFilledContourPlot(ctx, scale, data, thresholds, colors)
{
	var ratio0;
	var ratio1;

	if (thresholds.length != colors.length) {
		alert("Error: disparity of threshold and color arrays");
		return;
	}

	for (t = 0; t < thresholds.length; t++) {
		ctx.fillStyle = colors[t];

		for (i = 0; i < data.length - 1; i++) {
			for (j = 0 ; j < data[i].length - 1; j++) {
				var p0 = data[i][j];
				var p1 = data[i][j+1];
				var p2 = data[i+1][j+1];
				var p3 = data[i+1][j];

				var cellClass = classifyCell(p0, p1, p2, p3, thresholds[t]);
				switch (cellClass) {
					case 15: /* all square filled */
						ctx.beginPath();
						moveToScaledInv(ctx, scale, j, i);
						lineToScaledInv(ctx, scale, j+1, i);	
						lineToScaledInv(ctx, scale, j+1, i+1);
						lineToScaledInv(ctx, scale, j, i+1);
						lineToScaledInv(ctx, scale, j, i);
						ctx.closePath();
						ctx.fill();
						break;
					case 12: /* --- top filled */
						ratio0 = interpRatio(data[i][j], data[i+1][j], thresholds[t]);
						ratio1 = interpRatio(data[i][j+1], data[i+1][j+1], thresholds[t]);
						// console.log("12");
						
						ctx.beginPath();
						moveToScaledInv(ctx, scale, j, i+ratio0);
						lineToScaledInv(ctx, scale, j+1, i+ratio1);	
						lineToScaledInv(ctx, scale, j+1, i);
						lineToScaledInv(ctx, scale, j, i);
						lineToScaledInv(ctx, scale, j, i+ratio0);
						ctx.closePath();
						ctx.fill();
						break;
					case 3: /* --- bottom filled */
						ratio0 = interpRatio(data[i][j], data[i+1][j], thresholds[t]);
						ratio1 = interpRatio(data[i][j+1], data[i+1][j+1], thresholds[t]);
						// console.log("3");

						ctx.beginPath();
						moveToScaledInv(ctx, scale, j, i+ratio0);
						lineToScaledInv(ctx, scale, j+1, i+ratio1);
						lineToScaledInv(ctx, scale, j+1, i+1);
						lineToScaledInv(ctx, scale, j, i+1);
						lineToScaledInv(ctx, scale, j, i+ratio0);
						ctx.closePath();
						ctx.fill();
						break;
					case 9: /*  | left filled  */
						ratio0 = interpRatio(data[i][j], data[i][j+1], thresholds[t]);
						ratio1 = interpRatio(data[i+1][j], data[i+1][j+1], thresholds[t]);
						// console.log("9");

						ctx.beginPath();
						moveToScaledInv(ctx, scale, j+ratio0, i);
						lineToScaledInv(ctx, scale, j+ratio1, i+1);
						lineToScaledInv(ctx, scale, j, i+1);
						lineToScaledInv(ctx, scale, j, i);
						lineToScaledInv(ctx, scale, j+ratio0, i);
						ctx.closePath();
						ctx.fill();
						break;
					case 6: /*  | right filled */
						ratio0 = interpRatio(data[i][j], data[i][j+1], thresholds[t]);
						ratio1 = interpRatio(data[i+1][j], data[i+1][j+1], thresholds[t]);
						// console.log("6");

						ctx.beginPath();
						moveToScaledInv(ctx, scale, j+ratio0, i);
						lineToScaledInv(ctx, scale, j+ratio1, i+1);
						lineToScaledInv(ctx, scale, j+1, i+1);
						lineToScaledInv(ctx, scale, j+1, i);
						lineToScaledInv(ctx, scale, j+ratio0, i);
						ctx.closePath();
						ctx.fill();
						break;
					case 14: /* .\ right top filled */
						ratio0 = interpRatio(data[i][j], data[i+1][j], thresholds[t]);
						ratio1 = interpRatio(data[i+1][j], data[i+1][j+1], thresholds[t]);
						// console.log("14");

						ctx.beginPath();
						moveToScaledInv(ctx, scale, j, i+ratio0);
						lineToScaledInv(ctx, scale, j+ratio1, i+1);
						lineToScaledInv(ctx, scale, j+1, i+1);
						lineToScaledInv(ctx, scale, j+1, i);
						lineToScaledInv(ctx, scale, j, i);
						lineToScaledInv(ctx, scale, j, i+ratio0);
						ctx.closePath();
						ctx.fill();
						break;
					case 1: /* .\ left bottom filled */
						ratio0 = interpRatio(data[i][j], data[i+1][j], thresholds[t]);
						ratio1 = interpRatio(data[i+1][j], data[i+1][j+1], thresholds[t]);
						// console.log("1");

						ctx.beginPath();
						moveToScaledInv(ctx, scale, j, i+ratio0);
						lineToScaledInv(ctx, scale, j+ratio1, i+1);
						lineToScaledInv(ctx, scale, j, i+1);
						lineToScaledInv(ctx, scale, j, i+ratio0);
						ctx.closePath();
						ctx.fill();
						break;
					case 13: /*  /. left top filled */
						ratio0 = interpRatio(data[i+1][j], data[i+1][j+1], thresholds[t]);
						ratio1 = interpRatio(data[i][j+1], data[i+1][j+1], thresholds[t]);
						// console.log("13");

						ctx.beginPath();
						moveToScaledInv(ctx, scale, j+ratio0, i+1);
						lineToScaledInv(ctx, scale, j+1, i+ratio1);
						lineToScaledInv(ctx, scale, j+1, i);
						lineToScaledInv(ctx, scale, j, i);
						lineToScaledInv(ctx, scale, j, i+1);
						lineToScaledInv(ctx, scale, j+ratio0, i+1);
						ctx.closePath();
						ctx.fill();
						break;
					case 2: /*  /. right bottom filled */
						ratio0 = interpRatio(data[i+1][j], data[i+1][j+1], thresholds[t]);
						ratio1 = interpRatio(data[i][j+1], data[i+1][j+1], thresholds[t]);
						// console.log("2");

						ctx.beginPath();
						moveToScaledInv(ctx, scale, j+ratio0, i+1);
						lineToScaledInv(ctx, scale, j+1, i+ratio1);
						lineToScaledInv(ctx, scale, j+1, i+1);
						lineToScaledInv(ctx, scale, j+ratio0, i+1);
						ctx.closePath();
						ctx.fill();
						break;
					case 11: /*  \` left bottom filled */
						ratio0 = interpRatio(data[i][j], data[i][j+1], thresholds[t]);
						ratio1 = interpRatio(data[i][j+1], data[i+1][j+1], thresholds[t]);
						// console.log("11");

						ctx.beginPath();
						moveToScaledInv(ctx, scale, j+ratio0, i);
						lineToScaledInv(ctx, scale, j+1, i+ratio1);
						lineToScaledInv(ctx, scale, j+1, i+1);
						lineToScaledInv(ctx, scale, j, i+1);
						lineToScaledInv(ctx, scale, j, i);
						lineToScaledInv(ctx, scale, j+ratio0, i);
						ctx.closePath();
						ctx.fill();
						break;
					case 4: /*  \` right top filled */
						ratio0 = interpRatio(data[i][j], data[i][j+1], thresholds[t]);
						ratio1 = interpRatio(data[i][j+1], data[i+1][j+1], thresholds[t]);
						// console.log("4");

						ctx.beginPath();
						moveToScaledInv(ctx, scale, j+ratio0, i);
						lineToScaledInv(ctx, scale, j+1, i+ratio1);
						lineToScaledInv(ctx, scale, j+1, i);
						lineToScaledInv(ctx, scale, j+ratio0, i);
						ctx.closePath();
						ctx.fill();
						break;
					case 8: /* `/ left top filled */
						ratio0 = interpRatio(data[i][j], data[i+1][j], thresholds[t]);
						ratio1 = interpRatio(data[i][j], data[i][j+1], thresholds[t]);
						// console.log("8");

						ctx.beginPath();
						moveToScaledInv(ctx, scale, j, i+ratio0);
						lineToScaledInv(ctx, scale, j+ratio1, i);
						lineToScaledInv(ctx, scale, j, i);
						lineToScaledInv(ctx, scale, j, i+ratio0);
						ctx.closePath();
						ctx.fill();
						break;
					case 7: /* `/ right bottom filled */
						ratio0 = interpRatio(data[i][j], data[i+1][j], thresholds[t]);
						ratio1 = interpRatio(data[i][j], data[i][j+1], thresholds[t]);
						// console.log("7");

						ctx.beginPath();
						moveToScaledInv(ctx, scale, j, i+ratio0);
						lineToScaledInv(ctx, scale, j+ratio1, i);
						lineToScaledInv(ctx, scale, j+1, i);
						lineToScaledInv(ctx, scale, j+1, i+1);
						lineToScaledInv(ctx, scale, j, i+1);
						lineToScaledInv(ctx, scale, j, i+ratio0);
						ctx.closePath();
						ctx.fill();
						break;
					case 10: /* / / separate */
						// console.log("10");

						ratio0 = interpRatio(data[i][j], data[i+1][j], thresholds[t]);
						ratio1 = interpRatio(data[i][j], data[i][j+1], thresholds[t]);
						ctx.beginPath();
						moveToScaledInv(ctx, scale, j, i+ratio0);
						lineToScaledInv(ctx, scale, j+ratio1, i);
						lineToScaledInv(ctx, scale, j, i);
						lineToScaledInv(ctx, scale, j, i+ratio0);
						ctx.closePath();
						ctx.fill();

						ratio0 = interpRatio(data[i+1][j], data[i+1][j+1], thresholds[t]);
						ratio1 = interpRatio(data[i][j+1], data[i+1][j+1], thresholds[t]);
						ctx.beginPath();
						moveToScaledInv(ctx, scale, j+ratio0, i+1);
						lineToScaledInv(ctx, scale, j+1, i+ratio1);
						lineToScaledInv(ctx, scale, j+1, i+1);
						lineToScaledInv(ctx, scale, j+ratio0, i+1);
						ctx.closePath();
						ctx.fill();
						break;
					case 5: /* \ \ separate */
						// console.log("5");

						ratio0 = interpRatio(data[i][j], data[i+1][j], thresholds[t]);
						ratio1 = interpRatio(data[i+1][j], data[i+1][j+1], thresholds[t]);
						ctx.beginPath();
						moveToScaledInv(ctx, scale, j, i+ratio0);
						lineToScaledInv(ctx, scale, j+ratio1, i+1);
						lineToScaledInv(ctx, scale, j, i+1);
						lineToScaledInv(ctx, scale, j, i+ratio0);
						ctx.closePath();
						ctx.fill();

						ratio0 = interpRatio(data[i][j], data[i][j+1], thresholds[t]);
						ratio1 = interpRatio(data[i][j+1], data[i+1][j+1], thresholds[t]);
						ctx.beginPath();
						moveToScaledInv(ctx, scale, j+ratio0, i);
						lineToScaledInv(ctx, scale, j+1, i+ratio1);
						lineToScaledInv(ctx, scale, j+1, i);
						lineToScaledInv(ctx, scale, j+ratio0, i);
						ctx.closePath();
						ctx.fill();
						break;
				}
			}
		}
	}
}
