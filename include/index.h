const char MAIN_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
	<meta charset="UTF-8">
	<script>
		
		setInterval(function() {
			callEspFunction('getStats');
		}, 10000); //10.000 mSeconds update rate
		
		function callEspFunction(funcToCall) {
		 	var xhttp = new XMLHttpRequest();
		  	xhttp.onreadystatechange = function() {
		   	if (this.readyState == 4 && this.status == 200) {
		   		var datas = this.responseText.split(";");
		      	document.getElementById("Temp").innerHTML = datas[0];
		      	document.getElementById("Pres").innerHTML = datas[1];
		    	}
		  	};
		  	xhttp.open("GET", funcToCall, false);
		  	xhttp.send();
		}
	</script>
</head>
<style>
	body{
	    background: #020011;
	}
	
	.card{
	    background: #07007E;
	    width: 400px;
	    padding: 10px;
	    border-radius: 15px;
	    box-sizing: border-box;
	    color: #FFF;
	    margin:10px;
	    box-shadow: 0px 2px 18px -1px rgba(0,0,0,0.75);
	}
	
	h2,h1{
		line-height: 1;
		margin: 0px;	
	}
	
	button, input {
  		border: none;
  		width: 150px;
  		height: 50px;
  		color: white;
  		text-align: center;
  		margin: 3px 1px;
  		border-radius: 10px;
  		cursor: pointer;
  		outline: none;
	}
	
	.on{
  		background-color: darkgreen;	
	}
	
	.on:hover{
		background-color: green;	
	}
	
	.off{
  		background-color: darkred;		
	}
	
	.off:hover{
		background-color: red;	
	}
	
	.other{
  		background-color: darkgoldenrod;		
	}
	
	.other:hover{
		background-color: goldenrod;	
	}
	
	fieldset{
		border: 3px solid white;
	}
	
	hr.rounded {
		border-top: 3px solid white;
		border-radius: 3px;
	}
</style>
<body onload="callEspFunction('getStats')" id="body">
  	<fieldset class="card" style="width: 90%">

  	<legend><h1>Light set</h1></legend>

		 <form action="/get">
			Set light: <input class="card" type="number" id="light" name="light" min="0" max="255">
			<input class="on" type="submit" value="Submit">
		</form>

	</fieldset>

	<fieldset class="card">

		<legend><h1>BMP sensor</h1></legend>
	  	<h2>Temperature : <span id="Temp">0</span> C°</h2>
	  	<h2>Air pressure : <span id="Pres">0</span> hPa</h2>

	</fieldset>
</body>
</html>
)=====";