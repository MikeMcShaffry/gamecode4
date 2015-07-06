var SC=SC||{BUNDLE_INFO:{}};SC.browser=(function(){var c=navigator.userAgent.toLowerCase();
var a=(c.match(/.+(?:rv|it|ra|ie)[\/: ]([\d.]+)/)||[])[1];var b={version:a,safari:(/webkit/).test(c)?a:0,opera:(/opera/).test(c)?a:0,msie:(/msie/).test(c)&&!(/opera/).test(c)?a:0,mozilla:(/mozilla/).test(c)&&!(/(compatible|webkit)/).test(c)?a:0,mobileSafari:(/apple.*mobile.*safari/).test(c)?a:0,windows:!!(/(windows)/).test(c),mac:!!((/(macintosh)/).test(c)||(/(mac os x)/).test(c)),language:((navigator.language||navigator.browserLanguage).split("-",1)[0])};
b.current=(b.msie)?"msie":(b.mozilla)?"mozilla":(b.safari)?"safari":(b.opera)?"opera":"unknown";
return b})();SC.bundleDidLoad=function(a){var b=this.BUNDLE_INFO[a];if(!b){b=this.BUNDLE_INFO[a]={}
}b.loaded=true};SC.bundleIsLoaded=function(a){var b=this.BUNDLE_INFO[a];return b?!!b.loaded:false
};SC.loadBundle=function(){throw"SC.loadBundle(): SproutCore is not loaded."};SC.setupBodyClassNames=function(){var d=document.body;
var c,a,e,b,f;if(!d){return}c=SC.browser.current;a=(SC.browser.windows)?"windows":(SC.browser.mac)?"mac":"other-platform";
e=(document.documentElement.style.MozBoxShadow!==undefined)||(document.documentElement.style.webkitBoxShadow!==undefined)||(document.documentElement.style.oBoxShadow!==undefined)||(document.documentElement.style.boxShadow!==undefined);
b=(document.documentElement.style.MozBorderRadius!==undefined)||(document.documentElement.style.webkitBorderRadius!==undefined)||(document.documentElement.style.oBorderRadius!==undefined)||(document.documentElement.style.borderRadius!==undefined);
f=(d.className)?d.className.split(" "):[];if(e){f.push("box-shadow")}if(b){f.push("border-rad")
}f.push(c);f.push(a);if(SC.browser.mobileSafari){f.push("mobile-safari")}d.className=f.join(" ")
};if((typeof SC!=="undefined")&&SC&&SC.bundleDidLoad){SC.bundleDidLoad("sproutcore/bootstrap")
};