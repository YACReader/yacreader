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
					<li><img style="width: 92px" src="/images/f.png"/><a href="{element.url}">{element.name}</a> - <a href="{element.downloadurl}">Download</a></li>
				{end element}
				
				{loop elementcomic}
					<li><img style="width: 80px" src="{elementcomic.coverulr}" /> <a href="{elementcomic.url}">{elementcomic.name}</a></li>
				{end elementcomic}
			</ul>
			
			<div>{loop page} <a href="{page.url}"> {page.number} </a> {end page}</div>
					
	</div>
	<div class="sombra">&nbsp;</div>
</body>
</html>
