<html>

	<body>
		<h1>Libraries</h1>
		<p>
			<ul>
				{loop library}
					<li><a href="/library/{library.name}/folder/1">{library.label}</a></li>
				{end library}
			</ul>
		</p>
	</body>
</html>