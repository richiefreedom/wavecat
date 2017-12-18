var XMLHTTP;
var taskNum = 1;

function closeClick(e)
{
	var block = this.parentNode.parentNode;
	block.parentNode.removeChild(block);
}

function prepareCanvas(width, height)
{
	var plotContainer = document.getElementById("plotContainer");
	var panelInput = document.getElementById("panelInput");

	var block = document.createElement("div");
	var header = document.createElement("div");
	var close = document.createElement("div");
	var description = document.createElement("div");
	var result = document.createElement("div");
	var canvas = document.createElement("canvas");

	if (!canvas) {
		alert("Canvas is not supported by your web browser");
		return null;
	}

	canvas.width = width;
	canvas.height = height;

	close.className = "closeButton";
	close.innerHTML = "x";
	close.onclick = closeClick;

	header.innerHTML = "№" + taskNum++;
	header.className = "taskHeader";

	header.appendChild(close);

	description.innerHTML = "<pre>" + panelInput.value + "</pre>";
	description.className = "taskDescription";

	result.appendChild(canvas);

	block.appendChild(header);
	block.appendChild(description);
	block.appendChild(result);

	plotContainer.appendChild(block);
	plotContainer.scrollTop = plotContainer.scrollHeight;

	return canvas;
}

function drawPlot()
{
	var canvasWidth  = 3.0 * (experimentData.data.length) + 250;
	var canvasHeight = 3.0 * (experimentData.data[0].length) + 100;
	var canvas = prepareCanvas(canvasWidth, canvasHeight);
	if (!canvas)
		return;
	
	var panelContCheck = document.getElementById("panelContCheck");
	var panelMoreCheck = document.getElementById("panelMoreCheck");
	var ctx = canvas.getContext("2d");

	ctx.lineWidth = 1.0;

	var thresholds = [];
	var colors = [];
	var grayscale = [];

	var frequency = 0.2;
	var red;
	var green;
	var blue;

	var numThresholds = (panelMoreCheck.checked)? 16 : 8;

	ctx.fillStyle = "rgba(255, 255, 255, 1)";
	ctx.fillRect(0, 0, 800, 600);
	ctx.fillStyle = "rgba(0, 0, 0, 1)";
	
	ctx.font = "14px monospace";

	var delta = (experimentData.maxZ - experimentData.minZ) /
		(numThresholds - 0.95);
	for (i = 0; i < numThresholds; i++) {
		thresholds[i] = experimentData.minZ + 0.01 + i * delta;

		red   = 256 - (256/numThresholds * i) - 256/numThresholds;
		green = 256 - (256/numThresholds * i) - 256/numThresholds;
		blue  = 256 - (256/numThresholds * i) - 256/numThresholds;

		grayscale[i] = "#" + red.toString(16) + green.toString(16) + blue.toString(16);

		red   = 256/numThresholds * i + 256/numThresholds;
		green = 256/numThresholds * i + 256/numThresholds;
		blue  = 256/numThresholds * i + 256/numThresholds;

		colors[i] = "#" + red.toString(16) + green.toString(16) + blue.toString(16);
	}

	ctx.save();
	canvasTranslateScaled(ctx, 3.0, 5, 20);
	drawThresholdPalette(ctx, 3.0, thresholds, grayscale);
	ctx.restore();

	ctx.save();
	canvasTranslateScaled(ctx, 3.0, 50, 10);

	ctx.lineWidth = 1.0;

	drawFilledContourPlot(ctx, 3.0, experimentData.data, thresholds,
			grayscale);
	if (panelContCheck.checked)
		drawContourPlot(ctx, 3.0, experimentData.data, thresholds,
				colors);
	ctx.restore();

	ctx.save();
	canvasTranslateScaled(ctx, 3.0, 50, 10);
	drawAxesScaled(ctx, 3.0, 0, 0, experimentData);
	ctx.restore();
}

function ajaxCallback()
{
	var panelResponse = document.getElementById("panelResponse");
	var panelProgress = document.getElementById("panelProgress");

	if (XMLHTTP.readyState == 4) {
		switch (XMLHTTP.status) {
		case 200:
			var date = new Date();

			panelProgress.className = "hidden";

			panelResponse.value += date.getHours() + ":";
			panelResponse.value += date.getMinutes() + ":";
			panelResponse.value += date.getSeconds() + " ";

			if (XMLHTTP.responseText.length == 0) {
				panelResponse.value +=
					"unknown calculation error\n";
				return;
			} else if (-1 !=
				XMLHTTP.responseText.indexOf("Error")) {
				panelResponse.value +=
					"calculation error.\n";
				panelResponse.value += XMLHTTP.responseText;
				return;
			}

			eval(XMLHTTP.responseText);

			panelResponse.value +=
				"calculation finished successfully\n";

			drawPlot();

			break;
		}

		panelResponse.scrollTop = panelResponse.scrollHeight;
	}
}

function panelSubmitClick(e)
{
	var panelInput = document.getElementById("panelInput");
	var panelFileUpload = document.getElementById("panelFileUpload");
	var panelProgress = document.getElementById("panelProgress");
	var panelFileSlide = document.getElementById("panelFileSlide");
	var files = panelFileUpload.files;

	var formData = new FormData();

	if (panelFileSlide.className == "shown") {
		if (!files.length) {
			alert(configTranslations[configLanguage]["alertNoFileUpload"]);
			return;
		}
		for (var i = 0; i < files.length; i++) {
			var file = files[i];
			formData.append("file", file, file.name);
		}
	} else {
		formData.append("string", panelInput.value);
	}

	AJAXSendTextPost(XMLHTTP, "wavecat.exe", formData, ajaxCallback);

	panelProgress.className = "shown";
}

function panelSelectChange(e)
{
	var panelInput = document.getElementById("panelInput");

	if (this.selectedIndex != -1)
		panelInput.value = taskList[this.selectedIndex][1];
}

function panelChangeModeClick(e)
{
	var panelInputSlide = document.getElementById("panelInputSlide");
	var panelFileSlide = document.getElementById("panelFileSlide");
	var panelChangeMode = document.getElementById("panelChangeMode");
	var panelFileUpload = document.getElementById("panelFileUpload");

	if (panelInputSlide.className == "shown") {
		panelInputSlide.className = "hidden";
		panelFileSlide.className = "shown";
		panelChangeMode.value = configTranslations[configLanguage]["panelChangeModeFile"];
	} else {
		panelInputSlide.className = "shown";
		panelFileSlide.className = "hidden";
		panelChangeMode.value = configTranslations[configLanguage]["panelChangeModeInput"];
	}

}

function translate()
{
	var translation = configTranslations[configLanguage];
	if (translation) {
		for (var key in translation) {
			var obj = document.getElementById(key);
			if (obj) {
				obj.value = translation[key];
				obj.innerHTML = translation[key];
			}
		}
	}
}

function main(e)
{
	var panelSubmit = document.getElementById("panelSubmit");
	var panelSelect = document.getElementById("panelSelect");
	var panelChangeMode = document.getElementById("panelChangeMode");

	panelSubmit.onclick = panelSubmitClick;
	XMLHTTP = AJAXCreate();

	for (i = 0; i <  taskList.length; i++) {
		panelSelect.options[i] = new Option(taskList[i][0], taskList[i][1]);
	}
	panelSelect.selectedIndex = -1;
	panelSelect.onchange = panelSelectChange;

	panelChangeMode.onclick = panelChangeModeClick;

	translate();
}

window.onload = main;
