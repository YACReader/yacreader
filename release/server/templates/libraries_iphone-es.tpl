<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en" lang="en">
<head>	
	<link rel="stylesheet" href="/css/reset.css" type="text/css" />
	<link rel="stylesheet" href="/css/styles.css" type="text/css" />
	<title>Login</title>	
</head>
<body>
	<div id="content">
		<h1>BIBLIOTECAS</h1>
		<h2>Selecciona una biblioteca</h2>
		<p>
			<ul id="libraryList">
				{loop library}
					<li><a href="/library/{library.name}/folder/1">{library.label}</a></li>
				{end library}
			</ul>
		</p>
	</div>
	<div class="sombra">&nbsp;</div>
	</body>
</html>