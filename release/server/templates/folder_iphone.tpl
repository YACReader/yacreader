<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en" lang="en">
<head>
	<meta http-equiv="content-type" content="text/html; charset=utf-8" />
	<link rel="stylesheet" href="/css/reset.css" type="text/css" />
	<link rel="stylesheet" href="/css/styles.css" type="text/css" />
	<title>Folder</title>	
</head>
<body>
	<div id="content">
		<div>

			<div id="folder-header">
				<div id="topIndex">  {if pageIndex} <a class="previous" href="{page.previous}">previous</a> <a class="next" href="{page.next}">next</a> {end pageIndex} <a class="up" href="{upurl}">up</a>  </div>


				<div id="folder-subheader1">
				</div>

				<div id ="folder-subheader2">
					<a class="path" href="/">Libraries</a> <!--<img class="indicator" src="/images/indicator.png" style="width: 5px"/> <a class="path" href="{library.url}">{library.name}</a> -->{loop path}  <img class="indicator" src="/images/indicator.png" style="width: 5px"/> <a class="path" href="{path.url}">{path.name}</a> {end path} 
				</div>
				<div id ="header-combos">
					{if pageIndex}
						<img class="comboIndicator" src="/images/combo.png"/>
						<select name="indexnumber" id="indexnumber" onchange="navigateTo(this, 'window', false);">
							<option value="" disabled selected>{page}/{pages}</option>
							{loop page}
							<option value="{page.url}">{page.number}</option>
							{end page}
						</select>
					{end pageIndex}

					{if alphaIndex}
						<img class="comboIndicator" src="/images/combo.png"/>
						<select name="indexalpha" id="indexalpha" onchange="navigateTo(this, 'window', false);">
							<option value="" disabled selected>index</option>
							{loop index}
							<option value="{index.url}">{index.indexname}</option>
							{end index}
						</select>
					{end alphaIndex}

				</div>
			</div>

			<!--<a href="javascript:javascript:history.go(-1)">up</a>-->
			<div class="folderContent">
				<ul id="itemContainer">
					{loop element}
					<li>
						<div class="{element.class}">
							{element.cover.browse} <img style="width: 80px" src="{element.image.url}"/> {element.cover.browse.end}
						</div> 
						<div class="info"> 
							<div class="title"><p>{element.name}</p> 
							</div>
							<div class="elementInfo"> {element.pages} {element.size} 
							</div>
							<div class="buttons"> {element.download} {element.read} {element.browse}  
							</div>
						</div>
						{element.status}
					</li>
					{end element}
				</ul>
				<div class="clear">&nbsp;</div>
			</div>
		</div>

		{if index}
		<div id="indexes">
		{if alphaIndex}

		<div class="index">
			<ul id="alphaIndex">
				{loop index} 

				<li>  <a href="{index.url}"> {index.indexname} </a> </li>

				{end index}
			</ul>
			<div class="clear">&nbsp;</div>
		</div>

		{end alphaIndex}


		{if pageIndex}

		<div class="index">
			<ul id="pageIndex">
<!--				<li> <a class="first" href="{page.first}">first</a> </li>
				<li> <a class="previous" href="{page.previous}">previous</a> </li>-->

				{loop page} <li>  <a class="{page.current}" href="{page.url}"> {page.number} </a> </li> {end page} 

<!--				<li> <a class="next" href="{page.next}">next</a> </li>
				<li> <a class="last" href="{page.last}">last</a> </li>-->
			</ul>
			<div class="clear">&nbsp;</div>
		</div>
		{end pageIndex}
		</div>
		{end index}

		<script type="text/javascript">
		function navigateTo(sel, target) {
		    window[target].location.href = sel.options[sel.selectedIndex].value;
		}
		</script>

	</body>
	</html>
