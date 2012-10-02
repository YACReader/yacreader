<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en" lang="en">
<head>	
	<link rel="stylesheet" href="/css/reset.css" type="text/css" />
	<link rel="stylesheet" href="/css/styles.css" type="text/css" />
	<title>Login</title>	
</head>
<body>
	<div id="content">
		
		<h1>BROWSE AND IMPORT</h1>
		<h2>nombre folder?</h2>
		
		<a href="/">Libraries</a> <a href="{upurl}">up</a>
		<!--<a href="javascript:javascript:history.go(-1)">up</a>-->
		
			<ul id="itemContainer">
				{loop element}
					<li><img style="width: {element.image.width}" src="{element.image.url}"/> <p>{element.name}</p> {element.browse} - {element.download} </li>
				{end element}
			</ul>
			
			<div>{loop index} <a href="{index.url}"> {index.indexname} </a> {end index}</div>
			<br />
			<div>{loop page} <a href="{page.url}"> {page.number} </a> {end page}</div>
					
	</div>
	<div class="sombra">&nbsp;</div>
</body>
</html>
