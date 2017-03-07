// This file contains common JavaScript that is loaded into every page.
//

// Include hide/show script for SumoMe sharing widget attached to left side of browser
jQuery(document).scroll(function () {
  if(window.location.href.indexOf('/giftguide') != -1 ) return;
  var y = jQuery(this).scrollTop();
  if (y > 800) {
    jQuery('.sumome-share-client-wrapper-left-page').css({ opacity: 1 });
    jQuery('.sumome-share-client-wrapper-left-page').fadeIn();
  } else {
    jQuery('.sumome-share-client-wrapper-left-page').fadeOut();
  }
});
jQuery( "a.sumome-share-client-share" ).ready(function() {
  jQuery("a[title='Twitter']").addClass("SumoMeTwitter")
  jQuery("a[title='Facebook']").addClass("SumoMeFacebook")
  jQuery("a[title='Google+']").addClass("SumoMeGplus")
  jQuery("a[title='Reddit']").addClass("SumoMeReddit")
  jQuery("a[title='LinkedIn']").addClass("SumoMeLinkedin")
  jQuery("a[title='Pinterest']").addClass("SumoMePinterest")
});


// Open external links in new tab
jQuery(document).ready(function($) {
  $('a').not('[href*="mailto:"]').each(function() {
    var a = new RegExp('/' + window.location.host + '/');
    var href = this.href;
    if (!a.test(href)) {
      $(this).attr('target', '_blank');
    }
  });
});


// AddRoll Retargeting Pixel
adroll_adv_id = "KNRSJHIPMNCYTPL6CH6ZAM";
adroll_pix_id = "65IC7MDQJVEX5LW2ZNA7YF";
(function () {
  var oldonload = window.onload;
  window.onload = function(){
    __adroll_loaded=true;
    var scr = document.createElement("script");
    var host = (("https:" == document.location.protocol) ? "https://s.adroll.com" : "http://a.adroll.com");
    scr.setAttribute('async', 'true');
    scr.type = "text/javascript";
    scr.src = host + "/j/roundtrip.js";
    ((document.getElementsByTagName('head') || [null])[0] ||
    document.getElementsByTagName('script')[0].parentNode).appendChild(scr);
    if(oldonload){oldonload()}};
}());


// CrazyEgg
// setTimeout(function(){var a=document.createElement("script");
//   var b=document.getElementsByTagName("script")[0];
//   a.src=document.location.protocol+"//dnn506yrbagrg.cloudfront.net/pages/scripts/0013/2533.js?"+Math.floor(new Date().getTime()/3600000);
//   a.async=true;a.type="text/javascript";b.parentNode.insertBefore(a,b)
// }, 1);


// Begin Chartbeat Tracker
var _sf_async_config={};
/** CONFIGURATION START **/
_sf_async_config.uid = 53627;
_sf_async_config.domain = 'makezine.com';
_sf_async_config.useCanonical = true;
_sf_async_config.sections = 'Change this to your Section name';  //CHANGE THIS
_sf_async_config.authors = 'Change this to your Author name';    //CHANGE THIS
/** CONFIGURATION END **/
(function(){
  function loadChartbeat() {
    window._sf_endpt=(new Date()).getTime();
    var e = document.createElement('script');
    e.setAttribute('language', 'javascript');
    e.setAttribute('type', 'text/javascript');
    e.setAttribute('src', '//static.chartbeat.com/js/chartbeat.js');
    document.body.appendChild(e);
  }
  var oldonload = window.onload;
  window.onload = (typeof window.onload != 'function') ?
    loadChartbeat : function() { oldonload(); loadChartbeat(); };
})();
// End Chartbeat Tracker


// Load Typekit
try{Typekit.load();}catch(e){}
jQuery( document ).ready( function( $ ) {
    // Sadly the Facebook Comment Box does not allow us to change the positioning
    $( '.comment-list' ).appendTo( '#comments' );

    // Allow links within bootstrap tabs for sharing URL of a particular tab
    var url = document.location.toString();
    if ( url.match( '#' ) ) {
        $( '.nav-tabs a[href=#' + url.split( '#' )[1] + ']').tab( 'show' ) ;
    }

    // Change hash in URL for tab linking
    $( '.nav-tabs a' ).on( 'shown', function( e ) {
        // Use the History API if possible, or else, fall back.
        // The History API will allow us to hash a URL without page jump in modern browsers.
        if ( history.pushState ) {
            window.history.pushState( null, null, e.target.hash );
        } else {
            window.location.hash = e.target.hash;
        }
    });
});


// Fancybox subscribe modal stuff
jQuery(document).ready(function(){
  $(".fancybox-thx").fancybox({
    autoSize : false,
    width  : 400,
    autoHeight : true,
    padding : 0,
    afterLoad   : function() {
      this.content = this.content.html();
    }
  });
  // Footer Desktop
  $(document).on('submit', '.whatcounts-signup1', function (e) {
    e.preventDefault();
    var bla = $('#wc-email').val();
    $.post('http://whatcounts.com/bin/listctrl', $('.whatcounts-signup1').serialize());
    $('.fancybox-thx').trigger('click');
    $('.nl-modal-email-address').text(bla);
    $('.whatcounts-signup2 #email').val(bla);
  });
  // Footer Mobile
  $(document).on('submit', '.whatcounts-signup1m', function (e) {
    e.preventDefault();
    var bla = $('#wc-email-m').val();
    $.post('http://whatcounts.com/bin/listctrl', $('.whatcounts-signup1m').serialize());
    $('.fancybox-thx').trigger('click');
    $('.nl-modal-email-address').text(bla);
    $('.whatcounts-signup2 #email').val(bla);
  });
  // Sidebar
  $(document).on('submit', '.whatcounts-signup1s', function (e) {
    e.preventDefault();
    var bla = $('#wc-email').val();
    $.post('http://whatcounts.com/bin/listctrl', $('.whatcounts-signup1s').serialize());
    $('.fancybox-thx').trigger('click');
    $('.nl-modal-email-address').text(bla);
    $('.whatcounts-signup2 #email').val(bla);
  });
  // Header Overlay
  $(document).on('submit', '.whatcounts-signup1o', function (e) {
    e.preventDefault();
    var bla = $('#wc-email-o').val();
    $.post('http://whatcounts.com/bin/listctrl', $('.whatcounts-signup1o').serialize());
    $('.fancybox-thx').trigger('click');
    $('.nl-modal-email-address').text(bla);
    $('.whatcounts-signup2 #email').val(bla);
  });
  $(document).on('submit', '.whatcounts-signup2', function (e) {
    e.preventDefault();
    $.post('http://whatcounts.com/bin/listctrl', $('.fancybox-inner .whatcounts-signup2').serialize());
    $('.nl-thx-p1').hide();
    $('.nl-thx-p2').show();
  });
  $('input[type="checkbox"]').click(function(e){
    e.stopPropagation();
  });

  if(window.location.href.indexOf("?thankyou=true&subscribed-to=make-newsletter") > -1) {
    $(".fancybox-thx").fancybox({
      autoSize : false,
      width  : 400,
      autoHeight : true,
      padding : 0,
      afterLoad   : function() {
        this.content = this.content.html();
      }
    });
    $(".fancybox-thx").trigger('click');
  }
  else if(window.location.href.indexOf("?subscribed-to-make-newsletter") > -1) {
    $(".fancybox-thx").fancybox({
      autoSize : false,
      width  : 400,
      autoHeight : true,
      padding : 0,
      afterLoad   : function() {
        this.content = this.content.html();
      }
    });
    $(".fancybox-thx").trigger('click');
    $('.nl-thx-p1').hide();
    $('.nl-thx-p2').show();
  }
  else if(window.location.href.indexOf("?thankyou=true&subscribed-to=free-pdf") > -1) {
    $(".fancybox-thx-free-pdf").fancybox({
      autoSize : false,
      width  : 580,
      autoHeight : true,
      padding : 0,
      afterLoad   : function() {
        this.content = this.content.html();
      }
    });
    $(".fancybox-thx-free-pdf").trigger('click');
  }
  else if(window.location.href.indexOf("?success=true&subscribe-preferences") > -1) {
    $(".fancybox-sub-pref").fancybox({
      autoSize : false,
      width  : 480,
      autoHeight : true,
      padding : 0,
      afterLoad   : function() {
        this.content = this.content.html();
      }
    });
    $(".fancybox-sub-pref").trigger('click');
  }
  else if(window.location.href.indexOf("?thank-you-project-submission") > -1) {
    $(".fancybox-contribute-submission").fancybox({
      autoSize : false,
      width  : 480,
      autoHeight : true,
      padding : 0,
      afterLoad   : function() {
        this.content = this.content.html();
      }
    });
    $(".fancybox-contribute-submission").trigger('click');
  }
});


// Initialize the Lazyload function
jQuery(document).ready(function ($) {
  $('img.lazyload').lazyload({
    effect : "fadeIn"
  });
});


// Video Tracker
// note: this will cause the Youtube player to "flash" on the page when reloading to enable the JS API
for (var e = document.getElementsByTagName("iframe"), x = e.length; x--;)
  if (/youtube.com\/embed/.test(e[x].src))
    if(e[x].src.indexOf('enablejsapi=') === -1)
      e[x].src += (e[x].src.indexOf('?') ===-1 ? '?':'&') + 'enablejsapi=1';

var gtmYTListeners = []; // support multiple players on the same page
// attach our YT listener once the API is loaded
function onYouTubeIframeAPIReady() {
  for (var e = document.getElementsByTagName("iframe"), x = e.length; x--;) {
    if (/youtube.com\/embed/.test(e[x].src)) {
      gtmYTListeners.push(new YT.Player(e[x], {
        events: {
          onStateChange: onPlayerStateChange,
          onError: onPlayerError
        }
      }));
      YT.gtmLastAction = "p";
    }
  }
}
// listen for play/pause, other states such as rewind and end could also be added
// also report % played every second
function onPlayerStateChange(e) {
  e["data"] == YT.PlayerState.PLAYING && setTimeout(onPlayerPercent, 1000, e["target"]);
  var video_data = e.target["getVideoData"](),
    label = video_data.title;
  if (e["data"] == YT.PlayerState.PLAYING && YT.gtmLastAction == "p") {
    dataLayer.push({
      event: "videos",
      action: "play",
      label: label
    });
    YT.gtmLastAction = "";
  }
  if (e["data"] == YT.PlayerState.PAUSED) {
    dataLayer.push({
      event: "videos",
      action: "pause",
      label: label
    });
    YT.gtmLastAction = "p";
  }
}
// catch all to report errors through the GTM data layer
// once the error is exposed to GTM, it can be tracked in UA as an event!
function onPlayerError(e) {
  dataLayer.push({
    event: "error",
    action: "GTM",
    label: "videos:" + e["target"]["src"] + "-" + e["data"]
  })
}
// report the % played if it matches 0%, 25%, 50%, 75% or completed
function onPlayerPercent(e) {
  if (e["getPlayerState"]() == YT.PlayerState.PLAYING) {
    var t = e["getDuration"]() - e["getCurrentTime"]() <= 1.5 ? 1 : (Math.floor(e["getCurrentTime"]() / e["getDuration"]() * 4) / 4).toFixed(2);        if (!e["lastP"] || t > e["lastP"]) {
      var video_data = e["getVideoData"](),
        label = video_data.title;
      e["lastP"] = t;
      dataLayer.push({
        event: "videos",
        action: t * 100 + "%",
        label: label
      })
    }
    e["lastP"] != 1 && setTimeout(onPlayerPercent, 1000, e);
  }
}
// load the Youtube JS api and get going
var j = document.createElement("script"),
  f = document.getElementsByTagName("script")[0];
j.src = "//www.youtube.com/iframe_api";
j.async = true;
f.parentNode.insertBefore(j, f);


// YOUTUBE FOR FANCYBOX MODALS
jQuery(document).ready(function() {
  $(".fancytube").fancybox({
    maxWidth  : 800,
    maxHeight : 600,
    fitToView : false,
    width   : '70%',
    height    : '70%',
    autoSize  : false,
    closeClick  : false,
    openEffect  : 'none',
    closeEffect : 'none',
    padding : 0
  });
});
jQuery(document).ready(function() {
  $(".fancytube").fancybox();
});


// Page scrolling
jQuery(document).ready(function(){
  jQuery(".scroll").click(function(event){
    //prevent the default action for the click event
    event.preventDefault();
    //get the full url - like mysitecom/index.htm#home
    var full_url = this.href;
    //split the url by # and get the anchor target name - home in mysitecom/index.htm#home
    var parts = full_url.split("#");
    var trgt = parts[1];
    //get the top offset of the target anchor
    var target_offset = jQuery("#"+trgt).offset();
    var target_top = target_offset.top;
    //goto that anchor by setting the body scroll top to anchor top
    jQuery('html, body').animate({scrollTop:target_top - 50}, 1000);
    //Style the pagination links
    jQuery('a span.badge').addClass('badge-info');
  });
  jQuery('.hide-thumbnail').removeClass('thumbnail');
});


// Sitewide footer feedback form modal
$(".footer-feedback-btn").click(function() {
  $(".fancybox-feedback").fancybox({
    autoSize : false,
    width  : 560,
    autoHeight : true,
    padding : 0,
    openEffect : 'elastic',
    afterLoad   : function() {
      this.content = this.content.html();
    }
  });
  $(".fancybox-feedback").trigger('click');
});
// Feedback form submit event handler
$(document).on('submit', '#form13', function (e) {
  event.preventDefault();
  $.ajax({
    url:'//makemagazine.wufoo.com/forms/zzv65kl0b09v1f/#public',
    type:'POST',
    data:$(this).serialize()
  });
  $('.fancybox-feedback-inner1').hide();
  $('.fancybox-feedback-inner2').show();
});
