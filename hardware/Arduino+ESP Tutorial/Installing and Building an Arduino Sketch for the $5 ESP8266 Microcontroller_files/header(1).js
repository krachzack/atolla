jQuery( document ).ready( function( $ ) {


	// When clicking our search button, stop it from runnin a search and open the serach field and remove and add some classes
	$( '.search-make input[type="image"].disabled' ).click( function( event ) {

		if ( ! $(this).parent().hasClass( 'open' ) ) {
			event.preventDefault();

			// console.log( $(this) );
			$(this).removeClass( 'disabled' ).parent().addClass( 'open' ).children( '.search-field' ).css( 'display', 'inline-block' ).animate({
				width: '178px'
			});

			$( '.open .search-field' ).focus();
		}
	});

	// Track links clicked
	$( '.ga-nav a' ).click( function(e) {
		var link_name = $(this).text();
		var menu_name = $(this).parents('ul.nav').attr('id');

		// Track this click with Google, yo.
		ga('send', 'event', menu_name, 'Click', link_name);
	});

});
