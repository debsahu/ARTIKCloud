var ledcolor = {r: 255, g: 255, b: 255};
var wsc;

document.addEventListener("DOMContentLoaded", function(){
    wsc = new ReconnectingWebSocket('ws://' + window.location.hostname + '/ws');
    wsc.timeoutInterval = 3000;

    wsc.onopen = function (e) {
        console.log("WebSocketClient connected:", e);
    }
    wsc.onmessage = function (data, flags, number) {
        console.log('WebSocketClient message #${number}: ', data);
        var ledstatus = JSON.parse(data.data);
        ledcolor.r = ledstatus.colorRGB.r;
        ledcolor.g = ledstatus.colorRGB.g;
        ledcolor.b = ledstatus.colorRGB.b;
        colorPicker.color.rgb = ledcolor;
        document.getElementById("brightness").value = Math.round(ledstatus.level/ 2.55);
        document.getElementById("br").innerHTML = Math.round(ledstatus.level / 2.55);
        if(ledstatus.state == 1) {
            document.getElementById("onoff").checked = true;
        } else {
            document.getElementById("onoff").checked = false;
        }
    }

    document.getElementById("color_r").onchange = function (e) {
        var cv = e.target.value;
        var rgbc = colorPicker.color.rgb;
        colorPicker.color.rgb = { r: cv, g: rgbc.g, b: rgbc.b };
        var JsonData = { color: { r: rgbc.r, g: rgbc.g, b: rgbc.b } };
        wsc.send(JSON.stringify(JsonData));
    }

    document.getElementById("color_g").onchange = function (e) {
        var cv = e.target.value;
        var rgbc = colorPicker.color.rgb;
        colorPicker.color.rgb = { r: rgbc.r, g: cv, b: rgbc.b };
        var JsonData = { color: { r: rgbc.r, g: rgbc.g, b: rgbc.b } };
        wsc.send(JSON.stringify(JsonData));
    }

    document.getElementById("color_b").onchange = function (e) {
        var cv = e.target.value;
        var rgbc = colorPicker.color.rgb;
        colorPicker.color.rgb = { r: rgbc.r, g: rgbc.g, b: cv };
        var JsonData = { color: { r: rgbc.r, g: rgbc.g, b: rgbc.b } };
        wsc.send(JSON.stringify(JsonData));
    }

    document.getElementById("br").innerHTML = document.getElementById("brightness").value;

    document.getElementById("brightness").onchange = function (e) {
        document.getElementById("br").innerHTML = e.target.value;
        var JsonData = { level: Math.round(Number(e.target.value * 2.55)) };
        wsc.send(JSON.stringify(JsonData));
    }

    document.getElementById("onoff").onchange = function () {
        var JsonData;
        if (document.getElementById("onoff").checked) {
            JsonData = { state: true };
        } else {
            JsonData = { state: false };
        }
        wsc.send(JSON.stringify(JsonData));
    }
});

var colorPicker = new iro.ColorPicker("#color-picker-container", {
    width: 320,
    height: 320,
    color: { r: ledcolor.r, g: ledcolor.g, b: ledcolor.b },
    anticlockwise: true,
    borderWidth: 1,
    borderColor: "#fff",
    css: {
        "#swatch": {
            "background-color": "$color"
        }
    }
});

colorPicker.on("color:change", function (color) {
    document.getElementById("color_r").value = color.rgb.r;
    document.getElementById("color_g").value = color.rgb.g;
    document.getElementById("color_b").value = color.rgb.b;
    document.getElementById("values_r").innerHTML = color.rgb.r;
    document.getElementById("values_g").innerHTML = color.rgb.g;
    document.getElementById("values_b").innerHTML = color.rgb.b;
});

colorPicker.on("input:end", function (color) {
    var JsonData = { color: { r: color.rgb.r, g: color.rgb.g, b: color.rgb.b } };
    wsc.send(JSON.stringify(JsonData));
});