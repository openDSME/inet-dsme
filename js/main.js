jQuery(document).ready(function(){
	//cache DOM elements
    var sidebar = $('.cd-side-nav'),
		sidebarTrigger = $('.cd-nav-trigger');

	//mobile only - open sidebar when user clicks the menu
	sidebarTrigger.on('click', function(event){
		event.preventDefault();
		$([sidebar, sidebarTrigger]).toggleClass('nav-is-visible');
	});
});
