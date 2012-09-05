<html>
	<body>
		<h1>Folder</h1>
		<a href="{upurl}">up</a>

			<ul id="itemContainer">
				{loop element}
					<li><a href="{element.url}">{element.name}</a></li>
				{end element}
				
				{loop elementcomic}
					<li><img style="width: 100px" src="{elementcomic.coverulr}" /> <a href="{elementcomic.url}">{elementcomic.name}</a></li>
				{end elementcomic}
			</ul>
			
			<div>{loop page} <a href="{page.url}"> {page.number} </a> {end page}</div>
		
	</body>
</html>