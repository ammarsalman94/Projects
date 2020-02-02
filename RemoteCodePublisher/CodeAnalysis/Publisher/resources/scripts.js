function toggleStyle(elem) {		
	var oldlink = document.getElementsByTagName("link").item(0);	  
	var newlink = document.createElement("link");	  
	newlink.setAttribute("rel", "stylesheet");	  
	newlink.setAttribute("type", "text/css");    
	if(oldlink.getAttribute("href")=='resources/light.css'){  
		newlink.setAttribute("href", "resources/dark.css");
		elem.innerHTML = "Light Theme";
	}
	else {
		newlink.setAttribute("href", "resources/light.css");
		elem.innerHTML = "Dark Theme";
	}
	document.getElementsByTagName("head").item(0).replaceChild(newlink, oldlink);		
}  

function toggleminmax(elem, id) {
	var targetElem = document.getElementById(id);
	if(targetElem.style.display == "none"){
		targetElem.style.display = "inline";
		elem.innerHTML = "-";
		document.getElementById(id+"new").remove();
		document.getElementById(id+"hr").remove();
	}
	else {
		targetElem.style.display = "none";
		if(targetElem.innerHTML.substring(0,1)=='{'){
			var newElem = document.createElement("span");
			var txt = document.createTextNode("{...}");
			newElem.appendChild(txt);
			newElem.setAttribute("id", id+"new");
			newElem.style.border = "solid 1px";
			
			var line = document.createElement("hr");
			line.setAttribute("id", id+"hr");
			
			targetElem.parentNode.insertBefore(newElem, targetElem);
			targetElem.parentNode.insertBefore(line, targetElem);
		}
		else if(targetElem.childNodes[0].innerHTML.substring(0,2)=='/*'){
			var newElem = document.createElement("span");
			var txt = document.createTextNode("/* ... */");
			newElem.appendChild(txt);
			newElem.style.border = "solid 1px";
			newElem.setAttribute("class", "comment");
			newElem.setAttribute("id", id+"new");
			
			var line = document.createElement("hr");
			line.setAttribute("id", id+"hr");
			
			targetElem.parentNode.insertBefore(newElem, targetElem);
			targetElem.parentNode.insertBefore(line, targetElem);
			
		}
		elem.innerHTML = "+";
	}
}

function highlight(id){
	
	document.getElementById(id).style.backgroundColor = "gray";
}

function removeHighlight(id){
	document.getElementById(id).style.backgroundColor = "";
}