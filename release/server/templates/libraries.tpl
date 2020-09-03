<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en" lang="en">
<head>
	<meta http-equiv="content-type" content="text/html; charset=utf-8" />
	<link rel="stylesheet" href="/css/reset.css" type="text/css" />
	<link rel="stylesheet" href="/css/styles_{device}.css" type="text/css" />
	<title>Libraries</title>
</head>
<body>
	<div id="contentLibraries">
		<h1>Libraries</h1>
		<p>
			<ul id="librariesList">
				{loop library}
					<li>
					<div class="library-icon"> <img width="14px" height="18px" src="../images/library{display}.png" /> </div>
					<div class="library-link"> {library.label} </div>
					<div class="library-indicator"> <img width="5px" height="9px" src="../images/indicator{display}.png" /> </div>
					<a href="/library/{library.name}/folder/1">&nbsp;</a>
					<div class="clear">&nbsp;</div>
					</li>
				{end library}
			</ul>
		</p>
	</div>
	</body>
</html>
