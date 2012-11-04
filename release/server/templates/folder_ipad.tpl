<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en" lang="en">
<head>	
	<link rel="stylesheet" href="/css/reset.css" type="text/css" />
	<link rel="stylesheet" href="/css/styles.css" type="text/css" />
	<title>Folder</title>	
</head>
<body>
	<div id="content">
		
		<h1>BROWSE AND IMPORT</h1>
		<h2>{folder.name} {if pageIndex} - PAGE {page} OF {pages}{end pageIndex}</h2>
		
		<div id="topIndex"> {if pageIndex} <a class="next" href="{page.next}">next</a> <a class="previous" href="{page.previous}">previous</a> {end pageIndex} <a class="up" href="{upurl}">up</a>  <a class="libraries" href="/">Libraries</a>  </div>
		<!--<a href="javascript:javascript:history.go(-1)">up</a>-->
		<div class="folderContent">
			<ul id="itemContainer">
				{loop element}
					<li><div class="{element.class}"><img style="width: {element.image.width}" src="{element.image.url}"/></div> <div class="info"> <div class="title"><p>{element.name}</p> </div><div class="buttons"> {element.download} {element.browse}  </div></div></li>
				{end element}
			</ul>
			<div class="clear">&nbsp;</div>
		</div>
	</div>
	<div class="sombra">&nbsp;</div>
	
	{if alphaIndex}
	
	<div class="index">
	<ul id="alphaIndex">
	{loop index} 
		
		<li>  <a href="{index.url}"> {index.indexname} </a> </li>
	
	{end index}
	</ul>
	<div class="clear">&nbsp;</div>
	</div>
	<div class="sombra">&nbsp;</div>
	
	{end alphaIndex}
	
	
	{if pageIndex}
	
	<div class="index">
	<ul id="pageIndex">
		<li> <a class="first" href="{page.first}">first</a> </li>
		<li> <a class="previous" href="{page.previous}">previous</a> </li>
		
		{loop page} <li>  <a class="{page.current}" href="{page.url}"> {page.number} </a> </li> {end page} 
		
		<li> <a class="next" href="{page.next}">next</a> </li>
		<li> <a class="last" href="{page.last}">last</a> </li>
	</ul>
	<div class="clear">&nbsp;</div>
	</div>
	<div class="sombra">&nbsp;</div>
	{end pageIndex}
	
	
</body>
</html>
