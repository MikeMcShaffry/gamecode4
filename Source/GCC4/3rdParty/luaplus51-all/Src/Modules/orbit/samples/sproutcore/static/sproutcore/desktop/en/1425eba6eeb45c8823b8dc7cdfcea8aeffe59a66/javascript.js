SC.stringsFor("English",{"Invalid.CreditCard(%@)":"%@ is not a valid credit card number","Invalid.Email(%@)":"%@ is not a valid email address","Invalid.NotEmpty(%@)":"%@ must not be empty","Invalid.Password":"Your passwords do not match.  Please try typing them again.","Invalid.General(%@)":"%@ is invalid.  Please try again.","Invalid.Number(%@)":"%@ is not a number."});
SC.allowsBackspaceToPreviousPage=NO;SC.BORDER_BEZEL="sc-bezel-border";SC.BORDER_BLACK="sc-black-border";
SC.BORDER_GRAY="sc-gray-border";SC.BORDER_TOP="sc-top-border";SC.BORDER_BOTTOM="sc-bottom-border";
SC.BORDER_NONE=null;SC.Border={borderStyle:SC.BORDER_GRAY,_BORDER_REGEXP:(/-border$/),renderMixin:function(a,c){var b=this.get("borderStyle");
if(b){if(this._BORDER_REGEXP.exec(b)){a.addClass(b)}else{content.addStyle("border","1px %@ solid".fmt(b))
}}}};SC.CollectionGroup={classNames:["sc-collection-group"]};SC.CollectionRowDelegate={isCollectionRowDelegate:YES,rowHeight:18,customRowHeightIndexes:null,contentIndexRowHeight:function(a,b,c){return this.get("rowHeight")
}};SC.CollectionViewDelegate={isCollectionViewDelegate:YES,collectionViewSelectionForProposedSelection:function(a,b){return b
},collectionViewShouldSelectIndexes:function(a,b,c){return b},collectionViewShouldDeselectIndexes:function(a,b){return b
},collectionViewShouldDeleteIndexes:function(a,b){return b},collectionViewDeleteContent:function(a,c,b){if(!c){return NO
}if(SC.typeOf(c.destroyAt)===SC.T_FUNCTION){c.destroyAt(b);return YES}else{if(SC.typeOf(c.removeAt)===SC.T_FUNCTION){c.removeAt(b);
return YES}else{return NO}}},collectionViewShouldBeginDrag:function(a){return YES
},collectionViewDragDataTypes:function(a){return[]},collectionViewDragDataForType:function(a,c,b){return null
},collectionViewComputeDragOperations:function(a,b,c){return c},collectionViewValidateDragOperation:function(a,c,e,b,d){return(d&SC.DROP_ON)?SC.DRAG_NONE:e
},collectionViewPerformDragOperation:function(a,c,e,b,d){return SC.DRAG_NONE},collectionViewDragViewFor:function(a,b){return null
},ghostActsLikeCursor:NO};SC.Scrollable={isScrollable:true,verticalLineScroll:20,horizontalLineScroll:20,verticalPageScroll:function(){return this.get("innerFrame").height
}.property("innerFrame"),horizontalPageScroll:function(){return this.get("innerFrame").width
}.property("innerFrame"),hasVerticalScroller:function(){return this.get("scrollFrame").height>this.get("innerFrame").height
}.property("scrollFrame"),hasHorizontalScroller:function(){return this.get("scrollFrame").width>this.get("innerFrame").width
}.property("scrollFrame"),scrollBy:function(a){var b=this.get("scrollFrame");var c=this.get("innerFrame");
if(!this.get("hasVerticalScroller")){a.y=0}if(b.height<=c.height){a.y=0}if(!this.get("hasHorizontalScroller")){a.x=0
}if(b.width<=c.width){a.x=0}var d={x:b.x-(a.x||0),y:b.y-(a.y||0)};this.set("scrollFrame",d);
d=this.get("scrollFrame");return{x:d.x-b.x,y:d.y-b.y}},scrollTo:function(a,b){this.set("scrollFrame",{x:0-a,y:0-b})
},scrollToVisible:function(b){var e=this.get("innerFrame");var d=this.get("scrollFrame");
var a=this.convertFrameFromView(b.get("frame"),b);a.x-=(e.x+d.x);a.y-=(e.y+d.y);var c={x:0-d.x,y:0-d.y,width:e.width,height:e.height};
c.y-=Math.max(0,SC.minY(c)-SC.minY(a));c.x-=Math.max(0,SC.minX(c)-SC.minX(a));c.y+=Math.max(0,SC.maxY(a)-SC.maxY(c));
c.x+=Math.max(0,SC.maxX(a)-SC.maxX(c));this.scrollTo(c.x,c.y)},scrollDownLine:function(a){if(a===undefined){a=1
}return this.scrollBy({y:this.get("verticalLineScroll")*a}).y},scrollUpLine:function(a){if(a===undefined){a=1
}return 0-this.scrollBy({y:0-this.get("verticalLineScroll")*a}).y},scrollRightLine:function(a){if(a===undefined){a=1
}return this.scrollTo({y:this.get("horizontalLineScroll")*a}).x},scrollLeftLine:function(a){if(a===undefined){a=1
}return 0-this.scrollTo({y:0-this.get("horizontalLineScroll")*a}).x},scrollDownPage:function(a){if(a===undefined){a=1
}return this.scrollBy({y:this.get("verticalPageScroll")*a}).y},scrollUpPage:function(a){if(a===undefined){a=1
}return 0-this.scrollBy({y:0-this.get("verticalPageScroll")*a}).y},scrollRightPage:function(a){if(a===undefined){a=1
}return this.scrollTo({y:this.get("horizontalPageScroll")*a}).x},scrollLeftPage:function(a){if(a===undefined){a=1
}return 0-this.scrollTo({y:0-this.get("horizontalPageScroll")*a}).x}};SC.ModalPane=SC.Pane.extend({classNames:"sc-modal",layout:{top:0,left:0,bottom:0,right:0},_openPaneCount:0,paneWillAppend:function(a){this._openPaneCount++;
if(!this.get("isVisibleInWindow")){this.append()}return this},paneDidRemove:function(a){this._openPaneCount--;
if(this._openPaneCount<=0){this._openPaneCount=0;if(this.get("isVisibleInWindow")){this.remove()
}}},mouseDown:function(b){var a=this.get("owner");if(a&&a.modalPaneDidClick){a.modalPaneDidClick(b)
}}});sc_require("panes/modal");SC.PanelPane=SC.Pane.extend({layout:{left:0,right:0,top:0,bottom:0},classNames:["sc-panel"],acceptsKeyPane:YES,isModal:YES,modalPane:SC.ModalPane.extend({classNames:"for-sc-panel"}),contentView:null,contentViewBindingDefault:SC.Binding.single(),render:function(a,b){if(a.needsContent){this.renderChildViews(a,b);
a.push("<div class='top-left-edge'></div>","<div class='top-edge'></div>","<div class='top-right-edge'></div>","<div class='right-edge'></div>","<div class='bottom-right-edge'></div>","<div class='bottom-edge'></div>","<div class='bottom-left-edge'></div>","<div class='left-edge'></div>")
}},replaceContent:function(a){this.removeAllChildren();if(a){this.appendChild(a)}},createChildViews:function(){var a=this.contentView;
if(a){a=this.contentView=this.createChildView(a);this.childViews=[a]}},contentViewDidChange:function(){this.replaceContent(this.get("contentView"))
}.observes("contentView"),_modalPane:function(){var a=this.get("modalPane");if(a&&a.isClass){a=a.create({owner:this});
this.set("modalPane",a)}return a},appendTo:function(a){var b;if(!this.get("isVisibleInWindow")&&this.get("isModal")&&(b=this._modalPane())){this._isShowingModal=YES;
b.paneWillAppend(this)}return arguments.callee.base.apply(this,arguments)},remove:function(){var b,a=arguments.callee.base.apply(this,arguments);
if(this._isShowingModal){this._isShowingModal=NO;if(b=this._modalPane()){b.paneDidRemove(this)
}}return a},_isModalDidChange:function(){var b,a=this.get("isModal");if(a){if(!this._isShowingModal&&this.get("isVisibleInWindow")&&(b=this._modalPane())){this._isShowingModal=YES;
b.paneWillAppend(this)}}else{if(this._isShowingModal&&(b=this._modalPane())){this._isShowingModal=NO;
b.paneDidRemove(this)}}}.observes("isModal"),paneDidAttach:function(){var a=arguments.callee.base.apply(this,arguments);
this.get("rootResponder").makeKeyPane(this);return a},mouseDown:function(a){return YES
}});SC.TOGGLE_BEHAVIOR="toggle";SC.PUSH_BEHAVIOR="push";SC.TOGGLE_ON_BEHAVIOR="on";
SC.TOGGLE_OFF_BEHAVIOR="off";SC.ButtonView=SC.View.extend(SC.Control,SC.Button,SC.StaticLayout,{tagName:"a",classNames:["sc-button-view"],theme:"square",buttonBehavior:SC.PUSH_BEHAVIOR,isDefault:NO,isDefaultBindingDefault:SC.Binding.oneWay().bool(),isCancel:NO,isCancelBindingDefault:SC.Binding.oneWay().bool(),href:"",action:null,target:null,triggerAction:function(a){if(!this.get("isEnabled")){return NO
}this.set("isActive",YES);this._action(a);this.didTriggerAction();this.invokeLater("set",200,"isActive",NO);
return true},didTriggerAction:function(){},titleMinWidth:80,init:function(){arguments.callee.base.apply(this,arguments);
if(this.get("keyEquivalent")){this._defaultKeyEquivalent=this.get("keyEquivalent")
}},_TEMPORARY_CLASS_HASH:{},displayProperties:["href","icon","title","value","toolTip"],render:function(d,e){if(this.get("tagName")==="a"){var a=this.get("href");
if(!a||(a.length===0)){a="javascript:;"}d.attr("href",a)}var b=this.get("toolTip");
if(SC.typeOf(b)===SC.T_STRING){if(this.get("localize")){b=b.loc()}d.attr("title",b);
d.attr("alt",b)}var c=this._TEMPORARY_CLASS_HASH;c.def=this.get("isDefault");c.cancel=this.get("isCancel");
c.icon=!!this.get("icon");d.attr("role","button").setClass(c).addClass(this.get("theme"));
if(e){d=d.push("<span class='sc-button-inner' style = 'min-width:%@px'>".fmt(this.get("titleMinWidth")));
this.renderTitle(d,e);d.push("</span>")}else{this.renderTitle(d,e)}},_defaultKeyEquivalent:null,_isDefaultOrCancelDidChange:function(){var a=!!this.get("isDefault");
var b=!a&&this.get("isCancel");if(this.didChangeFor("defaultCancelChanged","isDefault","isCancel")){this.displayDidChange();
if(a){this.set("keyEquivalent","return")}else{if(b){this.setIfChanged("keyEquivalent","escape")
}else{this.set("keyEquivalent",this._defaultKeyEquivalent)}}}}.observes("isDefault","isCancel"),isMouseDown:false,mouseDown:function(a){if(!this.get("isEnabled")){return YES
}this.set("isActive",YES);this._isMouseDown=YES;return YES},mouseExited:function(a){if(this._isMouseDown){this.set("isActive",NO)
}return YES},mouseEntered:function(a){this.set("isActive",this._isMouseDown);return YES
},mouseUp:function(b){if(this._isMouseDown){this.set("isActive",NO)}this._isMouseDown=false;
var a=this.$().within(b.target);if(a&&this.get("isEnabled")){this._action(b)}return true
},keyDown:function(b){if(b.which===9){var a=b.shiftKey?this.get("previousValidKeyView"):this.get("nextValidKeyView");
a.becomeFirstResponder();return YES}if(b.which===13){this.triggerAction(b);return YES
}return YES},_action:function(a){switch(this.get("buttonBehavior")){case SC.TOGGLE_BEHAVIOR:var c=this.get("isSelected");
if(c){this.set("value",this.get("toggleOffValue"))}else{this.set("value",this.get("toggleOnValue"))
}break;case SC.TOGGLE_ON_BEHAVIOR:this.set("value",this.get("toggleOnValue"));break;
case SC.TOGGLE_OFF_BEHAVIOR:this.set("value",this.get("toggleOffValue"));break;default:var b=this.get("action");
var d=this.get("target")||null;if(b){if(this._hasLegacyActionHandler()){this._triggerLegacyActionHandler(a)
}else{this.getPath("pane.rootResponder").sendAction(b,d,this,this.get("pane"))}}}},_hasLegacyActionHandler:function(){var a=this.get("action");
if(a&&(SC.typeOf(a)==SC.T_FUNCTION)){return true}if(a&&(SC.typeOf(a)==SC.T_STRING)&&(a.indexOf(".")!=-1)){return true
}return false},_triggerLegacyActionHandler:function(evt){if(!this._hasLegacyActionHandler()){return false
}var action=this.get("action");if(SC.typeOf(action)==SC.T_FUNCTION){this.action(evt)
}if(SC.typeOf(action)==SC.T_STRING){eval("this.action = function(e) { return "+action+"(this, e); };");
this.action(evt)}},acceptsFirstResponder:function(){return this.get("isEnabled")}.property("isEnabled"),willBecomeKeyResponderFrom:function(a){if(!this._isFocused){this._isFocused=YES;
this.becomeFirstResponder();if(this.get("isVisibleInWindow")){this.$()[0].focus()
}}},willLoseKeyResponderTo:function(a){if(this._isFocused){this._isFocused=NO}}});
sc_require("panes/panel");sc_require("views/button");SC.BUTTON1_STATUS="button1";
SC.BUTTON2_STATUS="button2";SC.BUTTON3_STATUS="button3";SC.AlertPane=SC.PanelPane.extend({classNames:"sc-alert",delegate:null,icon:"sc-icon-alert-48",message:"",description:"",displayDescription:function(){var a=this.get("description");
if(!a||a.length===0){return a}a=SC.RenderContext.escapeHTML(a);return'<p class="description">'+a.split("\n").join('</p><p class="description">')+"</p>"
}.property("description").cacheable(),caption:"",displayCaption:function(){var a=this.get("caption");
if(!a||a.length===0){return a}a=SC.RenderContext.escapeHTML(a);return'<p class="caption">'+a.split("\n").join('</p><p class="caption">')+"</p>"
}.property("caption").cacheable(),buttonOne:SC.outlet("contentView.childViews.1.childViews.1"),buttonTwo:SC.outlet("contentView.childViews.1.childViews.0"),buttonThree:SC.outlet("contentView.childViews.2.childViews.0"),buttonThreeWrapper:SC.outlet("contentView.childViews.2"),layout:{centerX:0,width:500,top:55},contentView:SC.View.extend({useStaticLayout:YES,layout:{left:0,right:0,top:0,height:"auto"},childViews:[SC.View.extend(SC.StaticLayout,{classNames:["info"],render:function(a,d){var c=this.get("pane");
var b="/static/sproutcore/foundation/en/bcfdf5ca1125ccb312799938d8f09a81676f5db0/blank.gif";
if(c.get("icon")=="blank"){a.addClass("plain")}a.push('<img src="%@" class="icon %@" />'.fmt(b,c.get("icon")));
a.begin("h1").text(c.get("message")||"").end();a.push(c.get("displayDescription")||"");
a.push(c.get("displayCaption")||"");a.push('<div class="separator"></div>')}}),SC.View.extend({layout:{bottom:13,height:24,right:18,width:466},childViews:["cancelButton","okButton"],classNames:["textAlignRight"],cancelButton:SC.ButtonView.extend({useStaticLayout:YES,actionKey:SC.BUTTON2_STATUS,localize:YES,titleMinWidth:64,layout:{right:5,height:"auto",width:"auto",bottom:0},theme:"capsule",title:"Cancel",action:"dismiss",isVisible:NO}),okButton:SC.ButtonView.extend({useStaticLayout:YES,actionKey:SC.BUTTON1_STATUS,localize:YES,titleMinWidth:64,layout:{left:0,height:"auto",width:"auto",bottom:0},theme:"capsule",title:"OK",isDefault:YES,action:"dismiss"})}),SC.View.extend({layout:{bottom:13,height:24,left:18,width:150},isVisible:NO,childViews:[SC.ButtonView.extend({useStaticLayout:YES,actionKey:SC.BUTTON3_STATUS,localize:YES,titleMinWidth:64,layout:{left:0,height:"auto",width:"auto",bottom:0},theme:"capsule",title:"Extra",action:"dismiss",isVisible:NO})]})]}),dismiss:function(b){var a=this.delegate;
if(a&&a.alertPaneDidDismiss){a.alertPaneDidDismiss(this,b.get("actionKey"))}this.remove()
},alertInfoDidChange:function(){var a=this.getPath("contentView.childViews.0");if(a){a.displayDidChange()
}}.observes("icon","message","displayDescription","displayCaption")});SC.AlertPane._normalizeArguments=function(b){b=SC.A(b);
var a=b.length,c=b[a-1];if(SC.typeOf(c)!==SC.T_STRING){b[a-1]=null}else{c=null}b[7]=c;
return b};SC.AlertPane.show=function(p,l,n,b,c,o,a,g){var f=this._normalizeArguments(arguments);
var e=this.create({message:f[0]||"",description:f[1]||null,caption:f[2]||null,icon:f[6]||"sc-icon-alert-48",delegate:f[7]});
var k="buttonOne buttonTwo buttonThree".w(),d,h;for(var j=0;j<3;j++){d=e.get(k[j]);
h=f[j+3];if(h){d.set("title",h).set("isVisible",YES);if(j==2){var m=e.get("buttonThreeWrapper");
m.set("isVisible",YES)}}}var i=e.append();e.adjust("height",e.childViews[0].$().height());
e.updateLayout();return i};SC.AlertPane.warn=function(e,d,a,h,f,g,c){var b=this._normalizeArguments(arguments);
b[6]="sc-icon-alert-48";return this.show.apply(this,b)};SC.AlertPane.info=function(e,d,a,h,f,g,c){var b=this._normalizeArguments(arguments);
b[6]="sc-icon-info-48";return this.show.apply(this,b)};SC.AlertPane.error=function(e,d,a,h,f,g,c){var b=this._normalizeArguments(arguments);
b[6]="sc-icon-error-48";return this.show.apply(this,b)};SC.AlertPane.plain=function(e,d,a,h,f,g,c){var b=this._normalizeArguments(arguments);
b[6]="blank";return this.show.apply(this,b)};sc_require("panes/panel");SC.PalettePane=SC.PanelPane.extend({classNames:"sc-palette",isModal:NO,modalPane:SC.ModalPane,isAnchored:NO,_mouseOffsetX:null,_mouseOffsetY:null,mouseDown:function(a){var b=this.get("frame");
this._mouseOffsetX=b?(b.x-a.pageX):0;this._mouseOffsetY=b?(b.y-a.pageY):0},mouseDragged:function(a){if(!this.isAnchored){this.set("layout",{width:this.layout.width,height:this.layout.height,left:this._mouseOffsetX+a.pageX,top:this._mouseOffsetY+a.pageY});
this.updateLayout()}}});sc_require("panes/palette");SC.PICKER_MENU="menu";SC.PICKER_FIXED="fixed";
SC.PICKER_POINTER="pointer";SC.POINTER_LAYOUT=["perfectRight","perfectLeft","perfectTop","perfectBottom"];
SC.PickerPane=SC.PalettePane.extend({classNames:"sc-picker",isAnchored:YES,isModal:YES,pointerPos:"perfectRight",pointerPosX:0,pointerPosY:0,anchorElement:null,preferType:null,preferMatrix:null,popup:function(c,b,d){var a=c.isView?c.get("layer"):c;
this.beginPropertyChanges();this.set("anchorElement",a);if(b){this.set("preferType",b)
}if(d){this.set("preferMatrix",d)}this.endPropertyChanges();this.positionPane();this.append()
},positionPane:function(){var b=this.get("anchorElement"),c=this.get("preferType"),d=this.get("preferMatrix"),e=this.get("layout"),a;
if(b){b=this.computeAnchorRect(b);a=SC.cloneRect(b);if(c){switch(c){case SC.PICKER_MENU:case SC.PICKER_FIXED:if(!d||d.length!=3){this.set("preferMatrix",[1,4,3])
}a.x+=((this.preferMatrix[2]===0)?a.width:0)+this.preferMatrix[0];a.y+=((this.preferMatrix[2]===3)?a.height:0)+this.preferMatrix[1];
break;default:a.y+=a.height;break}}else{a.y+=a.height}a=this.fitPositionToScreen(a,this.get("frame"),b);
e={width:a.width,height:a.height,left:a.x,top:a.y}}else{e={width:e.width,height:e.height,centerX:0,centerY:0}
}this.set("layout",e).updateLayout();return this},computeAnchorRect:function(b){var a=SC.viewportOffset(b);
var c=SC.$(b);a.width=c.width();a.height=c.height();return a},fitPositionToScreen:function(e,c,b){var a=this.get("currentWindowSize")||SC.RootResponder.responder.computeWindowSize();
var d={x:0,y:0,width:a.width,height:a.height};c.x=e.x;c.y=e.y;if(this.preferType){switch(this.preferType){case SC.PICKER_MENU:c=this.fitPositionToScreenDefault(d,c,b);
c=this.fitPositionToScreenMenu(d,c);break;case SC.PICKER_POINTER:c=this.fitPositionToScreenPointer(d,c,b);
break;case SC.PICKER_FIXED:break;default:break}}else{c=this.fitPositionToScreenDefault(d,c,b)
}this.displayDidChange();return c},fitPositionToScreenDefault:function(c,d,b){if(SC.maxX(d)>c.width){var e=Math.max(SC.maxX(b),d.width);
d.x=Math.min(e,c.width)-d.width}if(SC.minX(d)<0){d.x=SC.minX(Math.max(b,0));if(SC.maxX(d)>c.width){d.x=Math.max(0,c.width-d.width)
}}if(SC.maxY(d)>c.height){e=Math.max((b.y-d.height),0);if(e>c.height){d.y=Math.max(0,c.height-d.height)
}else{d.y=e}}if(SC.minY(d)<0){e=Math.min(SC.maxY(b),(c.height-b.height));d.y=Math.max(e,0)
}return d},fitPositionToScreenMenu:function(a,b){if((b.x+b.width)>(a.width-20)){b.x=a.width-b.width-20
}if(b.x<7){b.x=7}if(b.height>a.height){b.y=15;b.height=a.height-35}return b},fitPositionToScreenPointer:function(n,h,l){var k=(l.height>12)?0:1;
var j=(l.height>12)?0:3;var d=[[l.x+l.width+(19+k),l.y+parseInt(l.height/2,0)-40],[l.x-h.width-(7+k),l.y+parseInt(l.height/2,0)-40],[l.x+parseInt(l.width/2,0)-parseInt(h.width/2,0),l.y-h.height-(17+j)],[l.x+parseInt(l.width/2,0)-parseInt(h.width/2,0),l.y+l.height+(17+j)]];
var c=[[l.x+l.width+h.width+(19+k),l.y+parseInt(l.height/2,0)+h.height-40],[l.x-(7+k),l.y+parseInt(l.height/2,0)+h.height-40],[l.x+parseInt(l.width/2,0)-parseInt(h.width/2,0)+h.width,l.y-(17+j)],[l.x+parseInt(l.width/2,0)-parseInt(h.width/2,0)+h.width,l.y+l.height+h.height+(17+j)]];
var e=[[d[0][1]>0?0:0-d[0][1],c[0][0]<n.width?0:c[0][0]-n.width,c[0][1]<n.height?0:c[0][1]-n.height,d[0][0]>0?0:0-d[0][0]],[d[1][1]>0?0:0-d[1][1],c[1][0]<n.width?0:c[1][0]-n.width,c[1][1]<n.height?0:c[1][1]-n.height,d[1][0]>0?0:0-d[1][0]],[d[2][1]>0?0:0-d[2][1],c[2][0]<n.width?0:c[2][0]-n.width,c[2][1]<n.height?0:c[2][1]-n.height,d[2][0]>0?0:0-d[2][0]],[d[3][1]>0?0:0-d[3][1],c[3][0]<n.width?0:c[3][0]-n.width,c[3][1]<n.height?0:c[3][1]-n.height,d[3][0]>0?0:0-d[3][0]]];
if(!this.preferMatrix||this.preferMatrix.length!=5){this.set("preferMatrix",[0,1,2,3,2])
}var b=this.preferMatrix;h.x=d[b[4]][0];h.y=d[b[4]][1];this.set("pointerPos",SC.POINTER_LAYOUT[b[4]]);
this.set("pointerPosX",0);this.set("pointerPosY",0);for(var g=0;g<SC.POINTER_LAYOUT.length;
g++){if(e[b[g]][0]===0&&e[b[g]][1]===0&&e[b[g]][2]===0&&e[b[g]][3]===0){if(b[4]!=b[g]){h.x=d[b[g]][0];
h.y=d[b[g]][1];this.set("pointerPos",SC.POINTER_LAYOUT[b[g]])}g=SC.POINTER_LAYOUT.length
}else{if((b[g]===0||b[g]===1)&&e[b[g]][0]===0&&e[b[g]][1]===0&&e[b[g]][2]<h.height-91&&e[b[g]][3]===0){if(b[4]!=b[g]){h.x=d[b[g]][0];
this.set("pointerPos",SC.POINTER_LAYOUT[b[g]])}h.y=d[b[g]][1]-e[b[g]][2];this.set("pointerPosY",e[b[g]][2]);
g=SC.POINTER_LAYOUT.length}else{if((b[g]===0||b[g]===1)&&e[b[g]][0]===0&&e[b[g]][1]===0&&e[b[g]][2]<=h.height-57&&e[b[g]][3]===0){if(b[4]!=b[g]){h.x=d[b[g]][0]
}h.y=d[b[g]][1]-(h.height-57);this.set("pointerPosY",(h.height-59));this.set("pointerPos",SC.POINTER_LAYOUT[b[g]]+" extra-low");
g=SC.POINTER_LAYOUT.length}}}}return h},render:function(b,d){var a=arguments.callee.base.apply(this,arguments);
if(b.needsContent){if(this.get("preferType")==SC.PICKER_POINTER){b.push('<div class="sc-pointer %@" style="margin-top: %@px"></div>'.fmt(this.get("pointerPos"),this.get("pointerPosY")))
}}else{var c=this.$(".sc-pointer");c.attr("class","sc-pointer %@".fmt(this.get("pointerPos")));
c.attr("style","margin-top: %@px".fmt(this.get("pointerPosY")))}return a},modalPaneDidClick:function(a){var b=this.get("frame");
if(!this.clickInside(b,a)){this.remove()}return YES},mouseDown:function(a){return this.modalPaneDidClick(a)
},clickInside:function(b,a){return SC.pointInRect({x:a.pageX,y:a.pageY},b)},windowSizeDidChange:function(b,a){arguments.callee.base.apply(this,arguments);
this.positionPane()}});SC.SeparatorView=SC.View.extend({classNames:["sc-separator-view"],tagName:"span",layoutDirection:SC.LAYOUT_HORIZONTAL,render:function(a,b){if(b){a.push("<span></span>")
}a.addClass(this.get("layoutDirection"))}});sc_require("views/button");sc_require("views/separator");
SC.BENCHMARK_MENU_ITEM_RENDER=YES;SC.MenuItemView=SC.ButtonView.extend(SC.ContentDisplay,{classNames:["sc-menu-item"],tagName:"div",parentPane:null,acceptsFirstResponder:YES,content:null,isSubMenuViewVisible:null,isSeparator:NO,contentValueKey:null,contentIsBranchKey:null,shortCutKey:null,contentIconKey:null,contentCheckboxKey:"checkbox",contentActionKey:null,itemWidth:100,itemHeight:20,subMenu:null,hasMouseExited:NO,anchor:null,displayProperties:["contentValueKey","contentIconKey","shortCutKey","contentIsBranchKey","itemHeight","subMenu","isEnabled","content"],contentDisplayProperties:"title value icon separator action checkbox shortcut branchItem subMenu".w(),render:function(b,a){var g;
if(SC.BENCHMARK_MENU_ITEM_RENDER){g="%@.render".fmt(this);SC.Benchmark.start(g)}var i=this.get("content");
var k=this.displayDelegate;var j,e;var d;var c=this.parentMenu();var h=this.get("itemWidth")||c.layout.width;
var f=this.get("itemHeight")||20;this.set("itemWidth",h);this.set("itemHeight",f);
if(!this.get("isEnabled")){b.addClass("disabled")}d=b.begin("a").attr("href","javascript: ;");
j=this.getDelegateProperty("isSeparatorKey",k);e=(j&&i)?(i.get?i.get(j):i[j]):null;
if(e){d.push("<span class='separator'></span>");b.addClass("disabled")}else{j=this.getDelegateProperty("contentCheckboxKey",k);
if(j){e=i?(i.get?i.get(j):i[j]):NO;if(e){d.begin("div").addClass("checkbox").end()
}}j=this.getDelegateProperty("contentIconKey",k);e=(j&&i)?(i.get?i.get(j):i[j]):null;
if(e&&SC.typeOf(e)!==SC.T_STRING){e=e.toString()}if(e){this.renderImage(d,e)}j=this.getDelegateProperty("contentValueKey",k);
e=(j&&i)?(i.get?i.get(j):i[j]):i;if(e&&SC.typeOf(e)!==SC.T_STRING){e=e.toString()
}this.renderLabel(d,e||"");j=this.getDelegateProperty("contentIsBranchKey",k);e=(j&&i)?(i.get?i.get(j):i[j]):NO;
if(e){this.renderBranch(d,e);d.addClass("has-branch")}else{j=this.getDelegateProperty("action",k);
e=(j&&i)?(i.get?i.get(j):i[j]):null;if(e&&isNaN(e)){this.set("action",e)}j=this.getDelegateProperty("target",k);
e=(j&&i)?(i.get?i.get(j):i[j]):null;if(e&&isNaN(e)){this.set("target",e)}if(this.getDelegateProperty("shortCutKey",k)){j=this.getDelegateProperty("shortCutKey",k);
e=(j&&i)?(i.get?i.get(j):i[j]):null;if(e){this.renderShortcut(d,e);d.addClass("shortcutkey")
}}}}d.end();if(SC.BENCHMARK_MENU_ITEM_RENDER){SC.Benchmark.end(g)}},renderImage:function(b,d){var a,c;
if(d&&SC.ImageView.valueIsUrl(d)){a=d;c=""}else{c=d;a="/static/sproutcore/foundation/en/bcfdf5ca1125ccb312799938d8f09a81676f5db0/blank.gif"
}b.begin("img").addClass("image").addClass(c).attr("src",a).end()},renderLabel:function(b,a){b.push("<span class='value'>"+a+"</span>")
},renderBranch:function(e,b){var c=">";var d="/static/sproutcore/foundation/en/bcfdf5ca1125ccb312799938d8f09a81676f5db0/blank.gif";
e.push('<span class= "hasBranch">'+c+"</span>")},renderShortcut:function(b,a){b.push('<span class = "shortcut">'+a+"</span>")
},getAnchor:function(){var a=this.get("anchor");if(a&&a.kindOf&&a.kindOf(SC.MenuItemView)){return a
}return null},isCurrent:NO,isSeparator:function(){var c=this.get("content");var a=this.displayDelegate;
var b=this.getDelegateProperty("isSeparatorKey",a);var d=(b&&c)?(c.get?c.get(b):c[b]):null;
if(d){return YES}return NO},isSubMenuAMenuPane:function(){var b=this.get("content");
var a=b.get("subMenu");if(a&&a.kindOf(SC.MenuPane)){return a}return NO},branching:function(){if(this.get("hasMouseExited")){this.set("hasMouseExited",NO);
return}this.createSubMenu()},loseFocus:function(){if(!this.isSubMenuAMenuPane()){this.set("hasMouseExited",YES);
this.$().removeClass("focus")}},createSubMenu:function(){var a=this.isSubMenuAMenuPane();
if(a){a.set("anchor",this);a.popup(this,[0,0,0]);var b=SC.RenderContext(this);b=b.begin(a.get("tagName"));
a.prepareContext(b,YES);b=b.end();var c=a.get("menuItemViews");if(c&&c.length>0){a.becomeKeyPane()
}}},parentMenu:function(){return this.get("parentPane")},isAnchorMouseDown:NO,mouseUp:function(a){if(!this.get("isEnabled")){this.set("hasMouseExited",NO);
return YES}this.set("hasMouseExited",NO);var c=this.get("contentCheckboxKey");var d=this.get("content");
if(c){if(d&&d.get(c)){d.set(c,NO)}else{if(d.get(c)!==undefined){d.set(c,YES)}}this.displayDidChange()
}this._action(a);var b=this.getAnchor();if(b){b.mouseUp(a)}else{this.resignFirstResponder()
}this.closeParent();return YES},mouseDown:function(a){return YES},mouseEntered:function(a){var b=this.parentMenu();
this.set("hasMouseExited",NO);if(b){b.becomeKeyPane();if(b.get("anchor")._isMouseDown){var e=b.getPath("anchor._isMouseDown");
this.set("isAnchorMouseDown",e);if(this.get("isAnchorMouseDown")){SC.Event.trigger(this.get("layer"),"mousedown")
}}}if(!this.get("isEnabled")&&!this.isSeparator()){return YES}var c=this.get("contentIsBranchKey");
if(c){var d=this.get("content");var f=(c&&d)?(d.get?d.get(c):d[c]):NO;if(f){this.invokeLater(this.branching(),100)
}}this.becomeFirstResponder();return YES},mouseExited:function(a){this.loseFocus();
var b=this.parentMenu();if(b){b.set("previousSelectedMenuItem",this)}return YES},moveUp:function(b,a){var c=this.parentMenu();
if(c){c.moveUp(this)}return YES},moveDown:function(b,a){var c=this.parentMenu();if(c){c.moveDown(this)
}return YES},moveRight:function(b,a){this.createSubMenu();return YES},keyDown:function(a){return this.interpretKeyEvents(a)
},keyUp:function(a){return YES},cancel:function(a){this.loseFocus();var b=this.parentMenu();
if(b){b.remove()}var c=b.getPath("anchor.pane");if(c){c.becomeKeyPane()}return YES
},didBecomeFirstResponder:function(a){if(a!==this){return}if(!this.isSeparator()){this.$().addClass("focus")
}var b=this.parentMenu();if(b){b.set("currentSelectedMenuItem",this)}},willLoseFirstResponder:function(a){if(a!==this){return
}this.$().removeClass("focus");var b=this.parentMenu();if(b){b.set("currentSelectedMenuItem",null);
b.set("previousSelectedMenuItem",this)}},insertNewline:function(b,a){this.mouseUp(a)
},closeParent:function(){this.$().removeClass("focus");var a=this.parentMenu();if(a){a.remove()
}},clickInside:function(b,a){return SC.pointInRect({x:a.pageX,y:a.pageY},b)}});require("panes/picker");
require("views/menu_item");SC.BENCHMARK_MENU_PANE_RENDER=YES;SC.MenuPane=SC.PickerPane.extend({menuItemKeys:"itemTitleKey itemValueKey itemIsEnabledKey itemIconKey itemSeparatorKey itemActionKey itemCheckboxKey itemShortCutKey itemBranchKey itemHeightKey subMenuKey itemKeyEquivalentKey itemTargetKey".w(),classNames:["sc-menu"],tagName:"div",isModal:YES,itemIsEnabledKey:"isEnabled",itemTitleKey:"title",items:[],itemValueKey:"value",itemIconKey:"icon",itemWidth:null,itemHeight:20,menuHeight:null,itemHeightKey:"height",subMenuKey:"subMenu",localize:YES,itemSeparatorKey:"separator",itemActionKey:"action",itemCheckboxKey:"checkbox",itemBranchKey:"branchItem",itemShortCutKey:"shortcut",itemKeyEquivalentKey:"keyEquivalent",itemTargetKey:"target",preferType:SC.PICKER_MENU,currentSelectedMenuItem:null,previousSelectedMenuItem:null,anchor:null,displayItemsArray:null,menuItemViews:[],exampleView:SC.MenuItemView,controlSize:SC.REGULAR_CONTROL_SIZE,createChildViews:function(){var e=[],a,c,b,d;
a=SC.MenuScrollView;b=this.get("menuItemViews");d=SC.View.design({layout:{top:0,left:0,minHeight:this.get("menuHeight")},classNames:"menuContainer",childViews:b});
this.set("itemWidth",this.get("layout").width||100);a=this.createChildView(a,{borderStyle:SC.BORDER_NONE,contentView:d});
this.childViews=[a]},popup:function(b,d){var a=b.isView?b.get("layer"):b;this.beginPropertyChanges();
var c=this.get("displayItems");this.set("anchorElement",a);this.set("anchor",b);this.set("preferType",SC.PICKER_MENU);
if(d){this.set("preferMatrix",d)}this.endPropertyChanges();this.append();this.positionPane();
this.becomeKeyPane()},displayItems:function(){var g=this.get("items"),d=this.get("localize"),l=null,e,j,f=[],m,h=g.get("length"),i,k,c=SC._menu_fetchKeys,b=SC._menu_fetchItem,a=0;
for(i=0;i<h;++i){k=g.objectAt(i);if(SC.none(k)){continue}e=SC.typeOf(k);m=f.length;
if(e===SC.T_STRING){f[m]=SC.Object.create({title:k.humanize().titleize(),value:k,isEnabled:YES,icon:null,isSeparator:null,action:null,isCheckbox:NO,menuItemNumber:i,isShortCut:NO,isBranch:NO,itemHeight:this.get("itemHeight"),subMenu:null,keyEquivalent:null,target:null});
a=a+this.get("itemHeight")}else{if(e!==SC.T_ARRAY){if(l===null){l=this.menuItemKeys.map(c,this)
}j=l.map(b,k);j[j.length]=i;if(!l[0]&&k.toString){j[0]=k.toString()}if(!l[1]){j[1]=k
}if(!l[2]){j[2]=YES}if(!j[9]){j[9]=this.get("itemHeight")}if(j[4]){j[9]=9}a=a+j[9];
if(d&&j[0]){j[0]=j[0].loc()}f[m]=SC.Object.create({title:j[0],value:j[1],isEnabled:j[2],icon:j[3],isSeparator:j[4]||NO,action:j[5],isCheckbox:j[6],isShortCut:j[7],menuItemNumber:i,isBranch:j[8],itemHeight:j[9],subMenu:j[10],keyEquivalent:j[11],target:j[12]})
}}}this.set("menuHeight",a);this.set("displayItemsArray",f);this.generateMenuItems(f);
return f}.property("items").cacheable(),itemsDidChange:function(){if(this._items){this._items.removeObserver("[]",this,this.itemContentDidChange)
}this._items=this.get("items");if(this._items){this._items.addObserver("[]",this,this.itemContentDidChange)
}this.itemContentDidChange()}.observes("items"),itemContentDidChange:function(){this.notifyPropertyChange("displayItems")
},displayProperties:["displayItems","value","controlSize"],render:function(a,c){if(SC.BENCHMARK_MENU_PANE_RENDER){var b="%@.render".fmt(this);
SC.Benchmark.start(b)}arguments.callee.base.apply(this,arguments);if(c){if(!this.get("isEnabled")){return
}a.addStyle("text-align","center");a.push("<div class='top-left-edge'></div>","<div class='top-edge'></div>","<div class='top-right-edge'></div>","<div class='right-edge'></div>","<div class='bottom-right-edge'></div>","<div class='bottom-edge'></div>","<div class='bottom-left-edge'></div>","<div class='left-edge'></div>")
}if(SC.BENCHMARK_MENU_PANE_RENDER){SC.Benchmark.end(b)}},menuHeightObserver:function(){var a=this.layout.height;
var b=this.get("menuHeight");if(a!==b){this.adjust("height",b).updateLayout()}}.observes("menuHeight"),generateMenuItems:function(h){if(!this.get("isEnabled")){return
}var l,i,k,g,a,e,f=[],c,d,j;c=h.length;d=SC.makeArray(h);for(j=0;j<c;++j){l=h[j];
i=l.get("action");k=l.get("menuItemNumber");a=l.get("itemHeight");e=this.get("itemWidth");
g=this.createChildView(this.exampleView,{owner:g,displayDelegate:g,parentPane:this,anchor:this.get("anchor"),isVisible:YES,contentValueKey:"title",contentIconKey:"icon",contentCheckboxKey:this.itemCheckboxKey,contentIsBranchKey:"branchItem",isSeparatorKey:"separator",shortCutKey:"shortCut",action:i,target:l.get("target"),layout:{top:0,left:0,width:e,height:a},isEnabled:l.get("isEnabled"),itemHeight:a,itemWidth:e,keyEquivalent:l.get("keyEquivalent"),controlSize:this.get("controlSize"),content:SC.Object.create({title:l.get("title"),value:l.get("value"),icon:l.get("icon"),separator:l.get("isSeparator"),action:i,checkbox:l.get("isCheckbox"),shortCut:l.get("isShortCut"),branchItem:l.get("isBranch"),subMenu:l.get("subMenu")}),rootElementPath:[k]});
f.push(g)}var b=this.childViews[0].contentView;b.replaceAllChildren(f);b.adjust("minHeight",this.get("menuHeight"));
this.set("menuItemViews",f)},previousSelectedMenuItemObserver:function(){var b=this.get("previousSelectedMenuItem");
if(b){var a=b.isSubMenuAMenuPane();if(a){a.remove()}}}.observes("previousSelectedMenuItem"),isAnchorMenuItemType:function(){var a=this.get("anchor");
return(a&&a.kindOf&&a.kindOf(SC.MenuItemView))},performKeyEquivalent:function(h,l){var i,f,a,m,e,d,k,g,j;
if(!this.get("isEnabled")){return NO}this.displayItems();i=this.get("displayItemsArray");
if(!i){return NO}if(h==="escape"){this.remove();var c=this.getPath("anchor.pane");
if(c){c.becomeKeyPane()}}f=i.length;for(j=0;j<f;++j){m=i[j];e=m.get("keyEquivalent");
d=m.get("action");k=m.get("isEnabled");g=m.get("target")||this;if(e==h&&k){var b=SC.RootResponder.responder.sendAction(d,g);
this.remove();return b}}return NO},mouseDown:function(a){return YES},mouseUp:function(a){this.remove();
var b=this.get("anchor");if(this.isAnchorMenuItemType()){this.sendEvent("mouseUp",a,b)
}return YES},moveDown:function(b){var a=this.getNextEnabledMenuItem(b);if(b){b.resignFirstResponder()
}a.becomeFirstResponder()},moveUp:function(b){var a=this.getPreviousEnabledMenuItem(b);
if(b){b.resignFirstResponder()}a.becomeFirstResponder();return YES},getPreviousEnabledMenuItem:function(a){var d,f,e=this.get("menuItemViews");
if(e){var c=e.length;var h=(e.indexOf(a)===-1)?c:e.indexOf(a);var i=h,g=NO,b=NO;while((!g||b)&&--h!==i){if(h===-1){h=c-1
}f=e[h];g=f.get("isEnabled");d=f.get("content");if(d){b=d.get(f.get("isSeparatorKey"))
}}return e[h]}},getNextEnabledMenuItem:function(a){var d,f,e=this.get("menuItemViews");
if(e){var c=e.length;var h=(e.indexOf(a)===-1)?0:e.indexOf(a);var i=h,g=NO,b=NO;while((!g||b)&&++h!==i){if(h===c){h=0
}f=e[h];g=f.get("isEnabled");d=f.get("content");if(d){b=d.get(f.get("isSeparatorKey"))
}}return e[h]}},modalPaneDidClick:function(b){var d,c,a,e=this.get("frame");a=this.get("currentSelectedMenuItem");
if(a){c=a.getAnchor();if(c){d=c.parentMenu();if(d.kindOf(SC.MenuPane)){d.modalPaneDidClick(b)
}}}if(!this.clickInside(e,b)){this.remove()}return YES},getMenuItem:function(b,c){var e,d,a;
e=this.get("displayItemsArray");d=this.get("menuItemViews");if(e&&d){a=e.get(b).indexOf(c);
if(a!==-1){return d[a]}else{return null}}else{return null}}});SC._menu_fetchKeys=function(a){return this.get(a)
};SC._menu_fetchItem=function(a){if(!a){return null}return this.get?this.get(a):this[a]
};sc_require("views/button");SC.SelectButtonView=SC.ButtonView.extend({objects:[],objectsBindingDefault:SC.Binding.multiple(),nameKey:null,sortKey:null,valueKey:null,iconKey:null,localize:YES,disableSort:YES,classNames:["select-button"],itemList:[],currentSelItem:null,itemIdx:null,value:null,checkboxEnabled:YES,_defaultVal:null,_defaultTitle:null,_defaultIcon:null,theme:"popup",displayProperties:["icon","value","controlSize"],preferMatrix:null,SELECT_BUTTON_SPRITE_WIDTH:32,CUSTOM_MENU_ITEM_HEIGHT:20,isSelectedBinding:"*menu.isVisibleInWindow",isDefaultPosition:NO,lastMenuWidth:null,customView:null,needsEllipsis:YES,leftAlign:function(){var b=0;
var a=this.get("controlSize");if(a===SC.SMALL_CONTROL_SIZE){b=-14}if(a===SC.REGULAR_CONTROL_SIZE){b=-16
}return b}.property("controlSize"),sortObjects:function(b){if(!this.get("disableSort")){var a=this.get("sortKey")||this.get("nameKey");
b=b.sort(function(d,c){if(a){d=d.get?d.get(a):d[a];c=c.get?c.get(a):c[a]}return(d<c)?-1:((d>c)?1:0)
})}return b},render:function(b,a){arguments.callee.base.apply(this,arguments);var k=this.layout.width;
if(a&&k){this.adjust({width:k-this.SELECT_BUTTON_SPRITE_WIDTH})}var m=this.get("objects");
m=this.sortObjects(m);var d=this.get("nameKey");var j=this.get("iconKey");var n=this.get("valueKey");
var f=this.get("checkboxEnabled");var l=this.get("value");var g=this.get("localize");
var e=[];var c=YES;var i=0;m.forEach(function(p){if(p){var o=d?(p.get?p.get(d):p[d]):p.toString();
o=g?o.loc():o;var q=j?(p.get?p.get(j):p[j]):null;if(SC.none(p[j])){q=null}var s=(n)?(p.get?p.get(n):p[n]):p;
if(!SC.none(l)&&!SC.none(s)){if(l===s){this.set("title",o);this.set("icon",q)}}if(s===this.value){this.set("itemIdx",i);
c=!f?NO:YES}else{c=NO}if(i===0){this._defaultVal=s;this._defaultTitle=o;this._defaultIcon=q
}var r=SC.Object.create({title:o,icon:q,value:s,isEnabled:YES,checkbox:c,action:this.displaySelectedItem});
e.push(r)}i+=1;this.set("itemList",e)},this);if(a){var h=this.get("value");if(SC.none(h)){this.set("value",this._defaultVal);
this.set("title",this._defaultTitle);this.set("icon",this._defaultIcon)}}this.changeSelectButtonPreferMatrix(this.itemIdx)
},_action:function(k){var e=this.$(".sc-button-label")[0];var h=this.get("layer").offsetWidth;
var f=e.scrollWidth;var l=this.get("lastMenuWidth");if(f){var i=e.offsetWidth;if(f&&i){h=h+f-i
}}if(!l||(h>l)){l=h}this.set("lastMenuWidth",l);var c=this.get("currentSelItem");
var g=this.get("itemList");var j=this.get("controlSize");var d=this.get("customView");
var a=d?d:SC.MenuItemView;var b=SC.MenuPane.create({classNames:["select-button"],items:g,exampleView:a,isEnabled:YES,preferType:SC.PICKER_MENU,itemHeightKey:"height",layout:{width:l},controlSize:j,itemWidth:l,contentView:SC.View.extend({})});
if(!b){return NO}b.popup(this,this.preferMatrix);b.set("currentSelectedMenuItem",c);
return YES},displaySelectedItem:function(){var i=this.parentMenu();var b=i.get("currentSelectedMenuItem");
var e=i.menuItemViews;var h,c;var a=0;if(b&&e){a=e.indexOf(b)}var f=i.get("anchor");
var d=i.get("items");var g=d.length;var j=null;while(!j&&(--g>=0)){h=!SC.none(d[g].title)?d[g].title:d.toString();
c=!SC.none(d[g].value)?d[g].value:h;if(h===this.get("value")&&(a===g)){j=d;f.set("value",c);
f.set("title",h)}}f.set("icon",this.get("icon")).set("currentSelItem",b).set("itemIdx",a)
},changeSelectButtonPreferMatrix:function(){var d=0;var b=this.get("itemIdx");var a=this.get("leftAlign");
var e;var c;if(this.get("isDefaultPosition")){e=[a,4,3];this.set("preferMatrix",e)
}else{if(b){d=b*this.CUSTOM_MENU_ITEM_HEIGHT}c=[a,-d,2];this.set("preferMatrix",c)
}},mouseDown:function(a){if(!this.get("isEnabled")){return YES}this.set("isActive",YES);
this._isMouseDown=YES;this.becomeFirstResponder();this._action();return YES},keyDown:function(a){if(this.interpretKeyEvents(a)){return YES
}else{arguments.callee.base.apply(this,arguments)}},interpretKeyEvents:function(a){if(a){if((a.keyCode===38||a.keyCode===40)){this._action()
}else{if(a.keyCode===27){this.resignFirstResponder()}}}return arguments.callee.base.apply(this,arguments)
}});sc_require("panes/panel");SC.SheetPane=SC.PanelPane.extend({classNames:"sc-sheet",init:function(){arguments.callee.base.apply(this,arguments)
}});SC.DRAG_LINK=4;SC.DRAG_COPY=1;SC.DRAG_MOVE=2;SC.DRAG_NONE=0;SC.DRAG_ANY=7;SC.DRAG_AUTOSCROLL_ZONE_THICKNESS=20;
SC.Drag=SC.Object.extend({source:null,ghostView:null,ghostActsLikeCursor:NO,dragView:null,ghost:YES,slideBack:YES,mouseDownEvent:null,ghostOffset:{x:0,y:0},location:{},dataTypes:function(){if(this.dataSource){return this.dataSource.get("dragDataTypes")||[]
}var d=this.data;if(d){var a=[];for(var b in d){if(d.hasOwnProperty(b)){a.push(b)
}}return a}var c=this.get("source");if(c&&c.dragDataTypes){return c.get("dragDataTypes")||[]
}return[]}.property().cacheable(),hasDataType:function(a){return(this.get("dataTypes").indexOf(a)>=0)
},dataForType:function(a){if(this.dataSource){return this.dataSource.dragDataForType(this,a)
}else{if(this.data){return this.data[a]}else{var b=this.get("source");if(b&&SC.typeOf(b.dragDataForType)==SC.T_FUNCTION){return b.dragDataForType(this,a)
}else{return null}}}},dataSource:null,data:null,allowedDragOperations:SC.DRAG_ANY,_dragInProgress:YES,startDrag:function(){this._createGhostView();
var n=this.event;var h={x:n.pageX,y:n.pageY};this.set("location",h);var b=this.dragView;
var d=b.get("pane");var o=b.get("parentView");var k=b.get("clippingFrame");var i=o?o.convertFrameToView(b.get("frame"),null):b.get("frame");
var j=d?d.get("frame"):{x:0,y:0};b.adjust({top:i.y+j.y,left:i.x+j.x,width:i.width,height:i.height});
var e=b.get("frame");var m=i;if(this.ghostActsLikeCursor){this.ghostOffset={x:14,y:14}
}else{this.ghostOffset={x:(h.x-m.x),y:(h.y-m.y)}}if(!this._ghostViewHidden){this._positionGhostView(n)
}this.ghostView.rootResponder.dragDidStart(this);var a=this.source;if(a&&a.dragDidBegin){a.dragDidBegin(this,h)
}var c=this._dropTargets();for(var l=0,g=c.length;l<g;l++){c[l].tryToPerform("dragStarted",this,n)
}},mouseDragged:function(a){var b=this._autoscroll(a);var f=this.get("location");
if(!b&&(a.pageX==f.x)&&(a.pageY==f.y)){return}f={x:a.pageX,y:a.pageY};this.set("location",f);
var d=this.source;var c=this._lastTarget;var e=this._findDropTarget(a);var g=SC.DRAG_NONE;
while(e&&(e!=c)&&(g==SC.DRAG_NONE)){if(e&&d&&d.dragSourceOperationMaskFor){g=d.dragSourceOperationMaskFor(this,e)
}else{g=SC.DRAG_ANY}if((g!=SC.DRAG_NONE)&&e&&e.computeDragOperations){g=g&e.computeDragOperations(this,a,g)
}else{g=SC.DRAG_NONE}this.allowedDragOperations=g;if(g==SC.DRAG_NONE){e=this._findNextDropTarget(e)
}}if(e!=c){if(c&&c.dragExited){c.dragExited(this,a)}if(e){if(e.dragEntered){e.dragEntered(this,a)
}if(e.dragUpdated){e.dragUpdated(this,a)}}this._lastTarget=e}else{if(e&&e.dragUpdated){e.dragUpdated(this,a)
}}if(d&&d.dragDidMove){d.dragDidMove(this,f)}if(!this._ghostViewHidden){this._positionGhostView(a)
}},mouseUp:function(l){var g={x:l.pageX,y:l.pageY},h=this._lastTarget,d=this.allowedDragOperations;
this.set("location",g);try{if(h&&h.acceptDragOperation&&h.acceptDragOperation(this,d)){d=h.performDragOperation?h.performDragOperation(this,d):SC.DRAG_NONE
}else{d=SC.DRAG_NONE}}catch(i){console.error("Exception in SC.Drag.mouseUp(acceptDragOperation|performDragOperation): %@".fmt(i))
}try{if(h&&h.dragExited){h.dragExited(this,l)}}catch(j){console.error("Exception in SC.Drag.mouseUp(target.dragExited): %@".fmt(j))
}var c=this._dropTargets();for(var k=0,f=c.length;k<f;k++){try{c[k].tryToPerform("dragEnded",this,l)
}catch(b){console.error("Exception in SC.Drag.mouseUp(dragEnded on %@): %@".fmt(c[k],b))
}}this._destroyGhostView();var a=this.source;if(a&&a.dragDidEnd){a.dragDidEnd(this,g,d)
}this._lastTarget=null;this._dragInProgress=NO},_createGhostView:function(){var b=this,c=this.dragView.get("frame"),a;
a=this.ghostView=SC.Pane.create({classNames:["sc-ghost-view"],layout:{top:c.y,left:c.x,width:c.width,height:c.height},owner:this,didCreateLayer:function(){if(b.dragView){var d=b.dragView.get("layer");
if(d){this.get("layer").appendChild(d.cloneNode(true))}}}});a.append()},_positionGhostView:function(a){var c=this.get("location");
c.x-=this.ghostOffset.x;c.y-=this.ghostOffset.y;var b=this.ghostView;if(b){b.adjust({top:c.y,left:c.x});
b.invokeOnce("updateLayout")}},_ghostViewHidden:NO,hideGhostView:function(){if(this.ghostView&&!this._ghostViewHidden){this.ghostView.remove();
this._ghostViewHidden=YES}},unhideGhostView:function(){if(this._ghostViewHidden){this._ghostViewHidden=NO;
this._createGhostView()}},_destroyGhostView:function(){if(this.ghostView){this.ghostView.remove();
this.ghostView=null;this._ghostViewHidden=NO}},_dropTargets:function(){if(this._cachedDropTargets){return this._cachedDropTargets
}var b=[];var d=SC.Drag._dropTargets;for(var c in d){if(d.hasOwnProperty(c)){b.push(d[c])
}}var f={};var e=SC.Drag._dropTargets;var a=function(g){if(!g){return 0}var i=SC.guidFor(g);
var h=f[i];if(!h){h=1;while(g=g.get("parentView")){if(e[SC.guidFor(g)]!==undefined){h++
}}f[i]=h}return h};b.sort(function(h,g){if(h===g){return 0}h=a(h);g=a(g);return(h>g)?-1:1
});this._cachedDropTargets=b;return b},_findDropTarget:function(c){var g={x:c.pageX,y:c.pageY};
var e,f;var d=this._dropTargets();for(var b=0,a=d.length;b<a;b++){e=d[b];if(!e.get("isVisibleInWindow")){continue
}f=e.convertFrameToView(e.get("clippingFrame"),null);if(SC.pointInRect(g,f)){return e
}}return null},_findNextDropTarget:function(a){var b=SC.Drag._dropTargets;while(a=a.get("parentView")){if(b[SC.guidFor(a)]){return a
}}return null},_autoscroll:function(l){if(!l){l=this._lastAutoscrollEvent}if(!this._dragInProgress){return NO
}var g=l?{x:l.pageX,y:l.pageY}:this.get("location"),h=this._findScrollableView(g),m=null,k,c,d,i,b,a,e;
while(h&&!m){k=h.get("canScrollVertical")?1:0;c=h.get("canScrollHorizontal")?1:0;
if(k||c){a=h.get("containerView");if(a){e=h.convertFrameToView(a.get("frame"),null)
}else{k=c=0}}if(k){i=SC.maxY(e);d=i-SC.DRAG_AUTOSCROLL_ZONE_THICKNESS;if(g.y>=d&&g.y<=i){k=1
}else{d=SC.minY(e);i=d+SC.DRAG_AUTOSCROLL_ZONE_THICKNESS;if(g.y>=d&&g.y<=i){k=-1}else{k=0
}}}if(c){i=SC.maxX(e);d=i-SC.DRAG_AUTOSCROLL_ZONE_THICKNESS;if(g.x>=d&&g.x<=i){c=1
}else{d=SC.minX(e);i=d+SC.DRAG_AUTOSCROLL_ZONE_THICKNESS;if(g.x>=d&&g.x<=i){c=-1}else{c=0
}}}if(k||c){m=h}else{h=this._findNextScrollableView(h)}}if(m&&(this._lastScrollableView===m)){if((Date.now()-this._hotzoneStartTime)>100){this._horizontalScrollAmount*=1.05;
this._verticalScrollAmount*=1.05}}else{this._lastScrollableView=m;this._horizontalScrollAmount=15;
this._verticalScrollAmount=15;this._hotzoneStartTime=(m)?Date.now():null;c=k=0}if(m&&(c||k)){var j={x:c*this._horizontalScrollAmount,y:k*this._verticalScrollAmount};
m.scrollBy(j)}if(m){if(l){this._lastAutoscrollEvent={pageX:l.pageX,pageY:l.pageY}
}this.invokeLater(this._autoscroll,100,null);return YES}else{this._lastAutoscrollEvent=null;
return NO}},_scrollableViews:function(){if(this._cachedScrollableView){return this._cachedScrollableView
}var a=[];var c=SC.Drag._scrollableViews;for(var b in c){if(c.hasOwnProperty(b)){a.push(c[b])
}}a=a.sort(function(f,d){var e=f;while(e=e.get("parentView")){if(d==e){return -1}}return 1
});this._cachedScrollableView=a;return a},_findScrollableView:function(f){var c=this._scrollableViews(),b=c?c.length:0,d,e,a;
for(a=0;a<b;a++){d=c[a];if(!d.get("isVisibleInWindow")){continue}e=d.convertFrameToView(d.get("clippingFrame"),null);
if(SC.pointInRect(f,e)){return d}}return null},_findNextScrollableView:function(a){var b=SC.Drag._scrollableViews;
while(a=a.get("parentView")){if(b[SC.guidFor(a)]){return a}}return null}});SC.Drag.mixin({start:function(b){var a=this.create(b);
a.startDrag();return a},_dropTargets:{},_scrollableViews:{},addDropTarget:function(a){this._dropTargets[SC.guidFor(a)]=a
},removeDropTarget:function(a){delete this._dropTargets[SC.guidFor(a)]},addScrollableView:function(a){this._scrollableViews[SC.guidFor(a)]=a
},removeScrollableView:function(a){delete this._scrollableViews[SC.guidFor(a)]}});
SC.MODIFIED_KEY_BINDINGS={"ctrl_.":"cancel",shift_tab:"insertBacktab",shift_left:"moveLeftAndModifySelection",shift_right:"moveRightAndModifySelection",shift_up:"moveUpAndModifySelection",shift_down:"moveDownAndModifySelection",alt_left:"moveLeftAndModifySelection",alt_right:"moveRightAndModifySelection",alt_up:"moveUpAndModifySelection",alt_down:"moveDownAndModifySelection",ctrl_a:"selectAll"};
SC.BASE_KEY_BINDINGS={escape:"cancel",backspace:"deleteBackward","delete":"deleteForward","return":"insertNewline",tab:"insertTab",left:"moveLeft",right:"moveRight",up:"moveUp",down:"moveDown",home:"moveToBeginningOfDocument",end:"moveToEndOfDocument",pagedown:"pageDown",pageup:"pageUp"};
SC.CAPTURE_BACKSPACE_KEY=NO;SC.PANEL_ORDER_LAYER=4096;SC.PALETTE_ORDER_LAYER=8192;
SC.POPUP_ORDER_LAYER=12288;SC.RootResponder=SC.RootResponder.extend({platform:"desktop",focusedPane:function(){var a=this.get("orderedPanes");
return a[a.length-1]}.property("orderedPanes"),orderedPanes:null,orderBefore:function(c,g){var a=this.get("focusedPane");
var h=this.get("orderedPanes").without(c);var f,i,d,e;var b=c.get("orderLayer");if(g){f=h.length;
i=h.indexOf(g);d=g.get("orderLayer");if(d<b){while((g.get("orderLayer")<b)&&(++i<f)){g=h[i]
}if(i>=f){g=null}}else{if(d>b){while((g.get("orderLayer")>b)&&(--i>=0)){g=h[i]}g=(i<0)?h[0]:h[i+1]
}}}else{i=h.length;while((--i>=0)&&!g){g=h[i];if(g.get("orderLayer")>b){g=null}}if(i<0){g=h[0]
}else{g=h[i+1]}}if(g){i=h.indexOf(g);h.insertAt(i,c)}else{h.push(c)}this.set("orderedPanes",h);
e=this.get("focusedPane");if(e!==a){if(a){a.blurTo(e)}if(e){e.focusFrom(a)}}return this
},orderOut:function(e){var d=this.get("focusedPane"),c=this.get("keyPane");var b=this.get("orderedPanes").without(e);
this.set("orderedPanes",b);if(d===e){var a=this.get("focusedPane");if(d){d.blurTo(a)
}if(a){a.focusFrom(d)}if(c===e){this.makeKeyPane(a)}}else{if(c===e){this.makeKeyPane(null)
}}return this},init:function(){arguments.callee.base.apply(this,arguments);this.orderedPanes=[]
},setup:function(){this.listenFor("keydown keyup mousedown mouseup click dblclick mouseout mouseover mousemove selectstart".w(),document).listenFor("resize focus blur".w(),window);
if(this.keypress){if(SC.CAPTURE_BACKSPACE_KEY&&SC.browser.mozilla){var b=this;document.onkeypress=function(c){c=SC.Event.normalizeEvent(c);
return b.keypress.call(b,c)};SC.Event.add(window,"unload",this,function(){document.onkeypress=null
})}else{SC.Event.add(document,"keypress",this,this.keypress)}}"drag selectstart".w().forEach(function(d){var e=this[d];
if(e){if(SC.browser.msie){var c=this;document.body["on"+d]=function(f){return e.call(c,SC.Event.normalizeEvent(event||window.event))
};SC.Event.add(window,"unload",this,function(){document.body["on"+d]=null})}else{SC.Event.add(document,d,this,e)
}}},this);var a=SC.browser.mozilla?"DOMMouseScroll":"mousewheel";SC.Event.add(document,a,this,this.mousewheel);
this.set("currentWindowSize",this.computeWindowSize());this.focus()},attemptKeyEquivalent:function(b){var e=null;
var d=b.commandCodes()[0];if(!d){return NO}var a=this.get("keyPane"),f=this.get("mainPane"),c=this.get("mainMenu");
if(a){e=a.performKeyEquivalent(d,b)}if(!e&&f&&(f!==a)){e=f.performKeyEquivalent(d,b)
}if(!e&&c){e=c.performKeyEquivalent(d,b)}return e},currentWindowSize:null,computeWindowSize:function(){var a;
if(window.innerHeight){a={width:window.innerWidth,height:window.innerHeight}}else{if(document.documentElement&&document.documentElement.clientHeight){a={width:document.documentElement.clientWidth,height:document.documentElement.clientHeight}
}else{if(document.body){a={width:document.body.clientWidth,height:document.body.clientHeight}
}}}return a},resize:function(){this._resize();return YES},_resize:function(){var a=this.computeWindowSize(),b=this.get("currentWindowSize");
this.set("currentWindowSize",a);if(!SC.rectsEqual(a,b)){if(this.panes){SC.RunLoop.begin();
this.panes.invoke("windowSizeDidChange",b,a);SC.RunLoop.end()}}},hasFocus:NO,focus:function(){if(!this.get("hasFocus")){SC.$("body").addClass("sc-focus").removeClass("sc-blur");
SC.RunLoop.begin();this.set("hasFocus",YES);SC.RunLoop.end()}return YES},blur:function(){if(this.get("hasFocus")){SC.$("body").addClass("sc-blur").removeClass("sc-focus");
SC.RunLoop.begin();this.set("hasFocus",NO);SC.RunLoop.end()}return YES},dragDidStart:function(a){this._mouseDownView=a;
this._drag=a},_lastModifiers:null,_handleModifierChanges:function(b){var a;a=this._lastModifiers=(this._lastModifiers||{alt:false,ctrl:false,shift:false});
var c=false;if(b.altKey!==a.alt){a.alt=b.altKey;c=true}if(b.ctrlKey!==a.ctrl){a.ctrl=b.ctrlKey;
c=true}if(b.shiftKey!==a.shift){a.shift=b.shiftKey;c=true}b.modifiers=a;return(c)?(this.sendEvent("flagsChanged",b)?b.hasCustomEventHandling:YES):YES
},_isFunctionOrNonPrintableKey:function(a){return !!(a.altKey||a.ctrlKey||a.metaKey||((a.charCode!==a.which)&&SC.FUNCTION_KEYS[a.which]))
},_isModifierKey:function(a){return !!SC.MODIFIER_KEYS[a.charCode]},keydown:function(a){if(SC.browser.mozilla>0&&(a.which===8)){return true
}var b=this._handleModifierChanges(a);var c=(a.which===8)&&!SC.allowsBackspaceToPreviousPage;
if(this._isModifierKey(a)){return(c?NO:b)}b=YES;if(this._isFunctionOrNonPrintableKey(a)){b=this.sendEvent("keyDown",a);
if(!b){b=!this.attemptKeyEquivalent(a)}else{b=a.hasCustomEventHandling;if(b){c=NO
}}}return c?NO:b},keypress:function(a){var b;if(SC.browser.mozilla>0&&(a.which===8)){b=this.sendEvent("keyDown",a);
return b?(SC.allowsBackspaceToPreviousPage||a.hasCustomEventHandling):YES}else{if(a.charCode!==undefined&&a.charCode===0){return YES
}return this.sendEvent("keyDown",a)?a.hasCustomEventHandling:YES}},keyup:function(a){var b=this._handleModifierChanges(a);
if(this._isModifierKey(a)){return b}return this.sendEvent("keyUp",a)?a.hasCustomEventHandling:YES
},mousedown:function(b){try{this.focus();this._clickCount+=1;if(!this._lastMouseUpAt||((Date.now()-this._lastMouseUpAt)>200)){this._clickCount=1
}b.clickCount=this._clickCount;var a=this.targetViewForEvent(b);a=this._mouseDownView=this.sendEvent("mouseDown",b,a);
if(a&&a.respondsTo("mouseDragged")){this._mouseCanDrag=YES}}catch(c){console.warn("Exception during mousedown: %@".fmt(c));
this._mouseDownView=null;this._mouseCanDrag=NO;throw c}return a?b.hasCustomEventHandling:YES
},mouseup:function(b){try{if(this._drag){this._drag.tryToPerform("mouseUp",b);this._drag=null
}var c=null,a=this._mouseDownView;this._lastMouseUpAt=Date.now();b.clickCount=this._clickCount;
if(a){c=this.sendEvent("mouseUp",b,a);if(!c&&(this._clickCount===2)){c=this.sendEvent("doubleClick",b,a)
}if(!c){c=this.sendEvent("click",b,a)}}if(!c){a=this.targetViewForEvent(b);if(this._clickCount===2){c=this.sendEvent("doubleClick",b,a)
}if(!c){c=this.sendEvent("click",b,a)}}this._mouseCanDrag=NO;this._mouseDownView=null
}catch(d){this._drag=null;this._mouseCanDrag=NO;this._mouseDownView=null;throw d}return(c)?b.hasCustomEventHandling:YES
},dblclick:function(a){if(SC.browser.isIE){this._clickCount=2;this.mouseup(a)}},mousewheel:function(b){try{var a=this.targetViewForEvent(b);
var c=this.sendEvent("mouseWheel",b,a)}catch(d){throw d}return(c)?b.hasCustomEventHandling:YES
},_lastHovered:null,mousemove:function(d){SC.RunLoop.begin();try{this.focus();if(this._drag){this._drag.tryToPerform("mouseDragged",d)
}else{var c=this._lastHovered||[];var f=[];var b=this.targetViewForEvent(d);var i;
while(b&&(b!==this)){if(c.indexOf(b)!==-1){b.tryToPerform("mouseMoved",d);f.push(b)
}else{b.tryToPerform("mouseEntered",d);f.push(b)}b=b.get("nextResponder")}for(var h=0,a=c.length;
h<a;h++){b=c[h];i=b.respondsTo("mouseExited");if(i&&!(f.indexOf(b)!==-1)){b.tryToPerform("mouseExited",d)
}}this._lastHovered=f;if(this._mouseDownView){this._mouseDownView.tryToPerform("mouseDragged",d)
}}}catch(g){throw g}SC.RunLoop.end()},_mouseCanDrag:YES,selectstart:function(b){var a=this.sendEvent("selectStart",b,this.targetViewForEvent(b));
return(a!=null?YES:NO)&&(this._mouseCanDrag?NO:YES)},drag:function(){return false
}});require("core");SC.UndoManager=SC.Object.extend({undoActionName:function(){return(this.undoStack)?this.undoStack.name:null
}.property("undoStack"),redoActionName:function(){return(this.redoStack)?this.redoStack.name:null
}.property("redoStack"),canUndo:function(){return this.undoStack!=null}.property("undoStack"),canRedo:function(){return this.redoStack!=null
}.property("redoStack"),undo:function(){this._undoOrRedo("undoStack","isUndoing")
},redo:function(){this._undoOrRedo("redoStack","isRedoing")},isUndoing:false,isRedoing:false,groupingLevel:0,registerUndo:function(b,a){this.beginUndoGroup(a);
this._activeGroup.actions.push(b);this.endUndoGroup(a)},beginUndoGroup:function(b){if(this._activeGroup){this.groupingLevel++
}else{var a=(this.isUndoing)?"redoStack":"undoStack";this._activeGroup={name:b,actions:[],prev:this.get(a)};
this.set(a,this._activeGroup);this.groupingLevel=1}},endUndoGroup:function(a){if(!this._activeGroup){raise("endUndoGroup() called outside group.")
}if(this.groupingLevel>1){this.groupingLevel--}else{this._activeGroup=null;this.groupingLevel=0
}this.propertyDidChange((this.isUndoing)?"redoStack":"undoStack")},setActionName:function(a){if(!this._activeGroup){raise("setActionName() called outside group.")
}this._activeGroup.name=a},_activeGroup:null,undoStack:null,redoStack:null,_undoOrRedo:function(a,c){if(this._activeGroup){return false
}if(this.get(a)==null){return true}this.set(c,true);var e=this.get(a);this.set(a,e.prev);
var b;var d=e.actions.length>1;if(d){this.beginUndoGroup(e.name)}while(b=e.actions.pop()){b()
}if(d){this.endUndoGroup(e.name)}this.set(c,false)}});SC.CheckboxView=SC.FieldView.extend(SC.StaticLayout,SC.Button,{classNames:["sc-checkbox-view"],tagName:"label",needsEllipsis:NO,render:function(a,f){var d,c;
if(f){d=this._field_currentDisplayTitle=this.get("displayTitle");var e="/static/sproutcore/foundation/en/bcfdf5ca1125ccb312799938d8f09a81676f5db0/blank.gif";
var b=this.get("isEnabled")?"":'disabled="disabled"';a.push('<span class="button" ></span>');
a.push('<input type="checkbox" name="%@" %@ />'.fmt(SC.guidFor(this),b));if(this.get("needsEllipsis")){a.push('<span class="label ellipsis">',d,"</span>")
}else{a.push('<span class="label">',d,"</span>")}a.attr("name",SC.guidFor(this))}else{if(c=this.$input()[0]){if(this.get("isEnabled")){c.disabled=NO
}else{c.disabled=YES}c=null}d=this.get("displayTitle");if(d!==this._field_currentDisplayTitle){this._field_currentDisplayTitle=d;
this.$("span.label").text(d)}}},$input:function(){return this.$("input")},getFieldValue:function(){var a=this.$input().attr("checked");
if(a){this._lastFieldValue=null}else{if(this._lastFieldValue===SC.MIXED_STATE){a=SC.MIXED_STATE
}}return a},setFieldValue:function(a){this._lastFieldValue=a;this.$input().attr("checked",(a===SC.MIXED_STATE)?NO:!!a)
},fieldValueForObject:function(a){return this.computeIsSelectedForValue(a)},objectForFieldValue:function(a){var b=(a===SC.MIXED_STATE)?this.get("value"):(!!a)?this.get("toggleOnValue"):this.get("toggleOffValue");
return b},didCreateLayer:function(){this.setFieldValue(this.get("fieldValue"));SC.Event.add(this.$input()[0],"click",this,this._field_fieldValueDidChange)
},willDestroyLayer:function(){SC.Event.remove(this.$input()[0],"click",this,this._field_fieldValueDidChange)
},mouseDown:function(a){return YES}});SC.LIST_ITEM_ACTION_CANCEL="sc-list-item-cancel-action";
SC.LIST_ITEM_ACTION_REFRESH="sc-list-item-cancel-refresh";SC.LIST_ITEM_ACTION_EJECT="sc-list-item-cancel-eject";
SC.ListItemView=SC.View.extend(SC.StaticLayout,SC.Control,{classNames:["sc-list-item-view"],content:null,hasContentIcon:NO,hasContentBranch:NO,contentCheckboxKey:null,contentIconKey:null,contentValueKey:null,escapeHTML:YES,contentUnreadCountKey:null,contentIsBranchKey:null,isEditing:NO,outlineIndent:16,outlineLevel:0,disclosureState:SC.LEAF_NODE,contentPropertyDidChange:function(){this.displayDidChange()
},render:function(c,a){var e=this.get("content"),i=this.displayDelegate,b=this.get("outlineLevel"),d=this.get("outlineIndent"),h,g,f;
f=c.begin("div").addClass("sc-outline");if(b>=0&&d>0){f.addStyle("left",d*(b+1))}g=this.get("disclosureState");
if(g!==SC.LEAF_NODE){this.renderDisclosure(f,g);c.addClass("has-disclosure")}h=this.getDelegateProperty("contentCheckboxKey",i);
if(h){g=e?(e.get?e.get(h):e[h]):NO;this.renderCheckbox(f,g);c.addClass("has-checkbox")
}if(this.getDelegateProperty("hasContentIcon",i)){h=this.getDelegateProperty("contentIconKey",i);
g=(h&&e)?(e.get?e.get(h):e[h]):null;this.renderIcon(f,g);c.addClass("has-icon")}h=this.getDelegateProperty("contentValueKey",i);
g=(h&&e)?(e.get?e.get(h):e[h]):e;if(g&&SC.typeOf(g)!==SC.T_STRING){g=g.toString()
}if(this.get("escapeHTML")){g=SC.RenderContext.escapeHTML(g)}this.renderLabel(f,g);
h=this.getDelegateProperty("contentUnreadCountKey",i);g=(h&&e)?(e.get?e.get(h):e[h]):null;
if(!SC.none(g)&&(g!==0)){this.renderCount(f,g)}h=this.getDelegateProperty("listItemActionProperty",i);
g=(h&&e)?(e.get?e.get(h):e[h]):null;if(g){this.renderAction(f,g);c.addClass("has-action")
}if(this.getDelegateProperty("hasContentBranch",i)){h=this.getDelegateProperty("contentIsBranchKey",i);
g=(h&&e)?(e.get?e.get(h):e[h]):NO;this.renderBranch(f,g);c.addClass("has-branch")
}c=f.end()},renderDisclosure:function(e,f){var d=(f===SC.BRANCH_OPEN)?"open":"closed",a=this._scli_disclosureHtml,c,b;
if(!a){a=this.constructor.prototype._scli_disclosureHtml={}}c=a[d];if(!c){c=a[d]='<img src="%@" class="disclosure button %@" />'.fmt(SC.BLANK_IMAGE_URL,d)
}e.push(c)},renderCheckbox:function(e,f){var d=(f===SC.MIXED_STATE)?"mixed":f?"sel":"nosel",a=this._scli_checkboxHtml,c,b;
if(!a){a=this.constructor.prototype._scli_checkboxHtml={}}c=a[d];if(!c){b=SC.RenderContext("a").attr("href","javascript:;").classNames(SC.CheckboxView.prototype.classNames);
if(f===SC.MIXED_STATE){b.addClass("mixed")}else{b.setClass("sel",f)}b.push('<span class="button"></span>');
c=a[d]=b.join()}e.push(c)},renderIcon:function(b,d){var a=null,c=null;if(d&&SC.ImageView.valueIsUrl(d)){a=d;
c=""}else{c=d;a=SC.BLANK_IMAGE_URL}b.begin("img").addClass("icon").addClass(c).attr("src",a).end()
},renderLabel:function(b,a){b.push("<label>",a||"","</label>")},$label:function(){return this.$("label")
},renderCount:function(a,b){a.push('<span class="count"><span class="inner">').push(b.toString()).push("</span></span>")
},renderAction:function(a,b){a.push('<img src="',SC.BLANK_IMAGE_URL,'" class="action" />')
},renderBranch:function(b,a){b.begin("span").addClass("branch").addClass(a?"branch-visible":"branch-hidden").push("&nbsp;").end()
},_isInsideElementWithClassName:function(e,a){var c=this.get("layer");if(!c){return NO
}var d=SC.$(a.target);var b=NO,f;while(!b&&d.length>0&&(d.get(0)!==c)){if(d.hasClass(e)){b=YES
}d=d.parent()}d=c=null;return b},_isInsideCheckbox:function(b){var a=this.displayDelegate;
var c=this.getDelegateProperty("contentCheckboxKey",a);return c&&this._isInsideElementWithClassName("sc-checkbox-view",b)
},_isInsideDisclosure:function(a){if(this.get("disclosureState")===SC.LEAF_NODE){return NO
}return this._isInsideElementWithClassName("disclosure",a)},mouseDown:function(a){if(this._isInsideCheckbox(a)){this._addCheckboxActiveState();
this._isMouseDownOnCheckbox=YES;this._isMouseInsideCheckbox=YES;return YES}else{if(this._isInsideDisclosure(a)){this._addDisclosureActiveState();
this._isMouseDownOnDisclosure=YES;this._isMouseInsideDisclosure=YES;return YES}}return NO
},mouseUp:function(h){var c=NO,i,d,b,a,g,f;if(this._isMouseDownOnCheckbox){if(this._isInsideCheckbox(h)){i=this.displayDelegate;
d=this.getDelegateProperty("contentCheckboxKey",i);b=this.get("content");if(b&&b.get){var e=b.get(d);
e=(e===SC.MIXED_STATE)?YES:!e;b.set(d,e);this.displayDidChange()}}this._removeCheckboxActiveState();
c=YES}else{if(this._isMouseDownOnDisclosure){if(this._isInsideDisclosure(h)){a=this.get("disclosureState");
g=this.get("contentIndex");f=(!SC.none(g))?SC.IndexSet.create(g):null;i=this.get("displayDelegate");
if(a===SC.BRANCH_OPEN){if(f&&i&&i.collapse){i.collapse(f)}else{this.set("disclosureState",SC.BRANCH_CLOSED)
}this.displayDidChange()}else{if(a===SC.BRANCH_CLOSED){if(f&&i&&i.expand){i.expand(f)
}else{this.set("disclosureState",SC.BRANCH_OPEN)}this.displayDidChange()}}}this._removeDisclosureActiveState();
c=YES}}this._isMouseInsideCheckbox=this._isMouseDownOnCheckbox=NO;this._isMouseDownOnDisclosure=this._isMouseInsideDisclosure=NO;
return c},mouseExited:function(a){if(this._isMouseDownOnCheckbox){this._removeCheckboxActiveState();
this._isMouseInsideCheckbox=NO}else{if(this._isMouseDownOnDisclosure){this._removeDisclosureActiveState();
this._isMouseInsideDisclosure=NO}}return NO},mouseEntered:function(a){if(this._isMouseDownOnCheckbox){this._addCheckboxActiveState();
this._isMouseInsideCheckbox=YES}else{if(this._isMouseDownOnDisclosure){this._addDisclosureActiveState();
this._isMouseInsideDisclosure=YES}}return NO},_addCheckboxActiveState:function(){var a=this.get("isEnabled");
this.$(".sc-checkbox-view").setClass("active",a)},_removeCheckboxActiveState:function(){this.$(".sc-checkbox-view").removeClass("active")
},_addDisclosureActiveState:function(){var a=this.get("isEnabled");this.$("img.disclosure").setClass("active",a)
},_removeDisclosureActiveState:function(){this.$("img.disclosure").removeClass("active")
},contentHitTest:function(b){var a=this.displayDelegate;var c=this.getDelegateProperty("contentValueKey",a);
if(!c){return NO}var e=this.$label().get(0);if(!e){return NO}var f=b.target,d=this.get("layer");
while(f&&(f!==d)&&(f!==window)){if(f===e){return YES}f=f.parentNode}return NO},beginEditing:function(){if(this.get("isEditing")){return YES
}return this._beginEditing(YES)},_beginEditing:function(s){var n=this.get("content"),g=this.get("displayDelegate"),e=this.getDelegateProperty("contentValueKey",g),h=this.get("parentView"),r=h?h.get("frame"):null,a=this.$label(),p,j,d,k,b,m,c,q,o,t;
if(s&&this.scrollToVisible()){var i=this.get("owner"),l=this.get("contentIndex");
this.invokeLater(function(){var f=i.itemViewForContentIndex(l);if(f&&f._beginEditing){f._beginEditing(NO)
}});return YES}if(!h||!a||a.get("length")===0){return NO}j=(e&&n&&n.get)?n.get(e):null;
p=this.computeFrameWithParentFrame(null);d=SC.viewportOffset(a[0]);k=a.css("lineHeight");
b=a.css("fontSize");m=this.$().css("top");if(m){m=parseInt(m.substring(0,m.length-2),0)
}else{m=0}c=k;q=0;if(b&&c){o=b*1.5;if(o<c){a.css({lineHeight:"1.5"});q=(c-o)/2}else{k=null
}}p.x=d.x;p.y=d.y+m+q;p.height=a[0].offsetHeight;p.width=(p.width-40-a[0].offsetLeft);
t=SC.InlineTextFieldView.beginEditing({frame:p,exampleElement:a,delegate:this,value:j,multiline:NO,isCollection:YES});
if(k){a.css({lineHeight:k})}return t},commitEditing:function(){if(!this.get("isEditing")){return YES
}return SC.InlineTextFieldView.commitEditing()},discardEditing:function(){if(!this.get("isEditing")){return YES
}return SC.InlineTextFieldView.discardEditing()},inlineEditorWillBeginEditing:function(a){this.set("isEditing",YES)
},inlineEditorDidBeginEditing:function(b){var a=this.$label();this._oldOpacity=a.css("opacity");
a.css("opacity",0)},inlineEditorShouldEndEditing:function(a,b){return YES},inlineEditorDidEndEditing:function(c,e){this.set("isEditing",NO);
var d=this.get("content");var a=this.displayDelegate;var b=this.getDelegateProperty("contentValueKey",a);
if(b&&d&&d.set){d.set(b,e)}this.displayDidChange()}});sc_require("mixins/collection_view_delegate");
sc_require("views/list_item");SC.DRAG_REORDER=16;SC.HORIZONTAL_ORIENTATION="horizontal";
SC.VERTICAL_ORIENTATION="vertical";SC.BENCHMARK_RELOAD=NO;SC.CollectionView=SC.View.extend(SC.CollectionViewDelegate,SC.CollectionContent,{classNames:["sc-collection-view"],ACTION_DELAY:200,content:null,contentBindingDefault:SC.Binding.multiple(),length:0,nowShowing:function(){return this.computeNowShowing()
}.property("length","clippingFrame").cacheable(),selection:null,isSelectable:YES,isSelectableBindingDefault:SC.Binding.bool(),isEnabled:YES,isEnabledBindingDefault:SC.Binding.bool(),isEditable:YES,isEditableBindingDefault:SC.Binding.bool(),canReorderContent:NO,canReorderContentBindingDefault:SC.Binding.bool(),canDeleteContent:NO,canDeleteContentBindingDefault:SC.Binding.bool(),canEditContent:NO,canEditContentBindingDefault:SC.Binding.bool(),isDropTarget:NO,useToggleSelection:NO,actOnSelect:NO,selectOnMouseDown:YES,exampleView:SC.ListItemView,contentExampleViewKey:null,groupExampleView:null,contentGroupExampleViewKey:null,action:null,target:null,contentValueKey:null,acceptsFirstResponder:NO,isActive:NO,computeLayout:function(){return null
},layoutForContentIndex:function(a){return null},allContentIndexes:function(){return SC.IndexSet.create(0,this.get("length")).freeze()
}.property("length").cacheable(),contentIndexesInRect:function(a){return null},computeNowShowing:function(){var c=this.contentIndexesInRect(this.get("clippingFrame"));
if(!c){c=this.get("allContentIndexes")}else{var b=this.get("length"),a=c.get("max");
if(a>b){c=c.copy().remove(b,a-b).freeze()}}return c},showInsertionPoint:function(a,b){},hideInsertionPoint:function(){},delegate:null,selectionDelegate:function(){var a=this.get("delegate"),b=this.get("content");
return this.delegateFor("isCollectionViewDelegate",a,b)}.property("delegate","content").cacheable(),contentDelegate:function(){var a=this.get("delegate"),b=this.get("content");
return this.delegateFor("isCollectionContent",a,b)}.property("delegate","content").cacheable(),contentRangeDidChange:function(d,b,c,a){if(!b&&(c==="[]")){this.reload(a)
}else{this.contentPropertyDidChange(b,c,a)}},contentPropertyDidChange:function(c,b,a){},updateContentRangeObserver:function(){var d=this.get("nowShowing"),a=this._cv_contentRangeObserver,c=this.get("content");
if(!c){return}if(a){c.updateRangeObserver(a,d)}else{var b=this.contentRangeDidChange;
a=c.addRangeObserver(d,this,b,null);this._cv_contentRangeObserver=a}},removeContentRangeObserver:function(){var b=this.get("content"),a=this._cv_contentRangeObserver;
if(a){if(b){b.removeRangeObserver(a)}this._cv_contentRangeObserver=null}},contentLengthDidChange:function(){var a=this.get("content");
this.set("length",a?a.get("length"):0)},_cv_contentDidChange:function(){var b=this.get("content"),a=this.contentLengthDidChange;
if(b===this._content){return this}this.removeContentRangeObserver();if(this._content){this._content.removeObserver("length",this,a)
}this._content=b;if(b){b.addObserver("length",this,a)}this.contentLengthDidChange();
this.contentRangeDidChange(b,null,"[]",null)}.observes("content"),_invalidIndexes:NO,reload:function(a){var b=this._invalidIndexes;
if(a&&b!==YES){if(b){b.add(a)}else{b=this._invalidIndexes=a.clone()}}else{this._invalidIndexes=YES
}if(this.get("isVisibleInWindow")){this.invokeOnce(this.reloadIfNeeded)}return this
},reloadIfNeeded:function(){var i=this._invalidIndexes;if(!i||!this.get("isVisibleInWindow")){return this
}this._invalidIndexes=NO;var h=this.get("content"),g=h?h.get("length"):0,f=this.computeLayout(),b=SC.BENCHMARK_RELOAD,a=this.get("nowShowing"),d=this._sc_itemViews,l=this.get("containerView")||this,n,m,c,k,j,e;
if(i.isIndexSet&&i.contains(a)){i=YES}if(this.willReload){this.willReload(i===YES?null:i)
}if(i.isIndexSet){j=l.get("childViews");c=j.get("length");if(b){SC.Benchmark.start(b="%@#reloadIfNeeded (Partial)".fmt(this),YES)
}i.forEach(function(o){var p=d?d[o]:null;if(a.contains(o)){k=this.itemViewForContentIndex(o,YES);
if(p&&p.parentView===l){e=p.get("layer");if(e&&e.parentNode){e.parentNode.removeChild(e)
}e=null;l.replaceChild(k,p)}else{l.appendChild(k)}}else{if(p&&p.parentView===l){delete d[o];
e=p.get("layer");if(e&&e.parentNode){e.parentNode.removeChild(e)}e=null;l.removeChild(p)
}}},this);if(b){SC.Benchmark.end(b)}}else{if(b){SC.Benchmark.start(b="%@#reloadIfNeeded (Full)".fmt(this),YES)
}if(d){d.length=0}n=[];a.forEach(function(o){n.push(this.itemViewForContentIndex(o,YES))
},this);l.beginPropertyChanges();l.destroyLayer().removeAllChildren();l.set("childViews",n);
l.replaceLayer();l.endPropertyChanges();if(b){SC.Benchmark.end(b)}}if(f){this.adjust(f)
}if(this.didReload){this.didReload(i===YES?null:i)}return this},displayProperties:"isFirstResponder isEnabled isActive".w(),render:function(a,b){if(b&&this._needsReload){this.reloadIfNeeded
}a.setClass("focus",this.get("isFirstResponder"));a.setClass("disabled",!this.get("isEnabled"));
a.setClass("active",this.get("isActive"));return arguments.callee.base.apply(this,arguments)
},_TMP_ATTRS:{},_COLLECTION_CLASS_NAMES:"sc-collection-item".w(),_GROUP_COLLECTION_CLASS_NAMES:"sc-collection-item sc-group-item".w(),itemViewForContentIndex:function(i,h){var f=this.get("content"),c=this._sc_itemViews,m=f.objectAt(i),l=this.get("contentDelegate"),g=l.contentGroupIndexes(this,f),b=NO,k,e,n,d,a;
if(!c){c=this._sc_itemViews=[]}if(!h&&(e=c[i])){return e}b=g&&g.contains(i);if(b){b=l.contentIndexIsGroup(this,f,i)
}if(b){k=this.get("contentGroupExampleViewKey");if(k&&m){n=m.get(k)}if(!n){n=this.get("groupExampleView")||this.get("exampleView")
}}else{k=this.get("contentExampleViewKey");if(k&&m){n=m.get(k)}if(!n){n=this.get("exampleView")
}}var j=this._TMP_ATTRS;j.contentIndex=i;j.content=m;j.owner=j.displayDelegate=this;
j.parentView=this.get("containerView")||this;j.page=this.page;j.layerId=this.layerIdFor(i,m);
j.isEnabled=l.contentIndexIsEnabled(this,f,i);j.isSelected=l.contentIndexIsSelected(this,f,i);
j.outlineLevel=l.contentIndexOutlineLevel(this,f,i);j.disclosureState=l.contentIndexDisclosureState(this,f,i);
j.isGroupView=b;j.isVisibleInWindow=this.isVisibleInWindow;if(b){j.classNames=this._GROUP_COLLECTION_CLASS_NAMES
}else{j.classNames=this._COLLECTION_CLASS_NAMES}d=this.layoutForContentIndex(i);if(d){j.layout=d
}else{delete j.layout}e=this.createItemView(n,i,j);c[i]=e;return e},_TMP_LAYERID:[],createItemView:function(c,a,b){return c.create(b)
},layerIdFor:function(a){var b=this._TMP_LAYERID;b[0]=SC.guidFor(this);b[1]=a;return b.join("-")
},contentIndexForLayerId:function(c){if(!c||!(c=c.toString())){return null}var b=this._baseLayerId;
if(!b){b=this._baseLayerId=SC.guidFor(this)+"-"}if((c.length<=b.length)||(c.indexOf(b)!==0)){return null
}var a=Number(c.slice(c.lastIndexOf("-")+1));return isNaN(a)?null:a},itemViewForEvent:function(j){var d=this.getPath("pane.rootResponder");
if(!d){return null}var c=SC.guidFor(this)+"-",a=c.length,e=j.target,g=this.get("layer"),f=null,b,i,h;
while(e&&e!==document&&e!==g){b=e?e.getAttribute("id"):null;if(b&&(f=this.contentIndexForLayerId(b))!==null){break
}e=e.parentNode}if(f===null||(e===g)){e=g=null;return null}if(f>=this.get("length")){throw"layout for item view %@ was found when item view does not exist (%@)".fmt(b,this)
}return this.itemViewForContentIndex(f)},expand:function(b){if(!b){return this}var a=this.get("contentDelegate"),c=this.get("content");
b.forEach(function(d){var e=a.contentIndexDisclosureState(this,c,d);if(e===SC.BRANCH_CLOSED){a.contentIndexExpand(this,c,d)
}},this);return this},collapse:function(b){if(!b){return this}var a=this.get("contentDelegate"),c=this.get("content");
b.forEach(function(d){var e=a.contentIndexDisclosureState(this,c,d);if(e===SC.BRANCH_OPEN){a.contentIndexCollapse(this,c,d)
}},this);return this},_cv_selectionDidChange:function(){var c=this.get("selection"),b=this._cv_selection,a=this._cv_selectionContentDidChange;
if(c===b){return this}if(b){b.removeObserver("[]",this,a)}if(c){c.addObserver("[]",this,a)
}this._cv_selection=c;this._cv_selectionContentDidChange()}.observes("selection"),_cv_selectionContentDidChange:function(){var c=this.get("selection"),b=this._cv_selindexes,a=this.get("content"),d;
this._cv_selindexes=c?c.frozenCopy():null;if(b){b=b.indexSetForSource(a)}if(c){c=c.indexSetForSource(a)
}if(c&&b){d=c.without(b).add(b.without(c))}else{d=c||b}if(d&&d.get("length")>0){this.reloadSelectionIndexes(d)
}},_invalidSelection:NO,reloadSelectionIndexes:function(a){var b=this._invalidSelection;
if(a&&(b!==YES)){if(b){b.add(a)}else{b=this._invalidSelection=a.copy()}}else{this._invalidSelection=YES
}if(this.get("isVisibleInWindow")){this.invokeOnce(this.reloadSelectionIndexesIfNeeded)
}return this},reloadSelectionIndexesIfNeeded:function(){var e=this._invalidSelection;
if(!e||!this.get("isVisibleInWindow")){return this}var d=this.get("nowShowing"),b=this._invalidIndexes,a=this.get("content"),c=this.get("selection");
this._invalidSelection=NO;if(b===YES||!d){return this}if(e===YES){e=d}if(b&&b.isIndexSet){e=e.without(b)
}e.forEach(function(f){if(!d.contains(f)){return}var g=this.itemViewForContentIndex(f,NO);
if(g){g.set("isSelected",c?c.contains(a,f):NO)}},this);return this},select:function(d,g){var e=this.get("content"),b=this.get("selectionDelegate"),a=this.get("contentDelegate"),c=a.contentGroupIndexes(this,e),f;
if(!this.get("isSelectable")){return this}if(SC.typeOf(d)===SC.T_NUMBER){d=SC.IndexSet.create(d,1)
}if(d&&d.get("length")>0){if(c&&c.get("length")>0){d=d.copy().remove(c)}d=b.collectionViewShouldSelectIndexes(this,d,g);
if(!d||d.get("length")===0){return this}}else{d=null}if(g&&(f=this.get("selection"))){f=f.copy()
}else{f=SC.SelectionSet.create()}if(d){f.add(e,d)}f=b.collectionViewSelectionForProposedSelection(this,f);
if(!f){f=SC.SelectionSet.create()}this._selectionAnchor=null;this.set("selection",f.freeze());
return this},deselect:function(b){var d=this.get("selection"),c=this.get("content"),a=this.get("selectionDelegate");
if(!this.get("isSelectable")){return this}if(!d||d.get("length")===0){return this
}if(SC.typeOf(b)===SC.T_NUMBER){b=SC.IndexSet.create(b,1)}b=a.collectionViewShouldDeselectIndexes(this,b);
if(!b||b.get("length")===0){return this}d=d.copy().remove(c,b);d=a.collectionViewSelectionForProposedSelection(this,d);
if(!d){d=SC.SelectionSet.create()}this.set("selection",d.freeze());return this},_findNextSelectableItemFromIndex:function(i,a){var d=this.get("length"),e=SC.IndexSet.create(),g=this.get("content"),j=this.get("selectionDelegate"),b=this.get("contentDelegate"),h=b.contentGroupIndexes(this,g),f,c;
if(!h&&(j.collectionViewShouldSelectIndexes===this.collectionViewShouldSelectIndexes)){return i
}while(i<d){if(!h||!h.contains(i)){e.add(i);f=j.collectionViewShouldSelectIndexes(this,e);
if(f&&f.get("length")>=1){return i}e.remove(i)}i++}if(a===undefined){c=this.get("selection");
a=c?c.get("max"):-1}return a},_findPreviousSelectableItemFromIndex:function(g,h){var c=SC.IndexSet.create(),d=this.get("content"),i=this.get("selectionDelegate"),a=this.get("contentDelegate"),f=a.contentGroupIndexes(this,d),e;
if(SC.none(g)){g=-1}if(!f&&(i.collectionViewShouldSelectIndexes===this.collectionViewShouldSelectIndexes)){return g
}while(g>=0){if(!f||!f.contains(g)){c.add(g);e=i.collectionViewShouldSelectIndexes(this,c);
if(e&&e.get("length")>=1){return g}c.remove(g)}g--}if(h===undefined){var b=this.get("selection");
h=b?b.get("min"):-1}if(SC.none(h)){h=-1}return h},selectPreviousItem:function(h,b){if(SC.none(b)){b=1
}if(SC.none(h)){h=false}var f=this.get("selection"),e=this.get("content");if(f){f=f.indexSetForSource(e)
}var g=f?f.get("min"):-1,a=f?f.get("max")-1:-1,d=this._selectionAnchor;if(SC.none(d)){d=g
}if(h){if(a>d){a=a-b}else{g=this._findPreviousSelectableItemFromIndex(g-b)}if(SC.none(g)||(g<0)){g=0
}if(a<g){a=g}}else{g=this._findPreviousSelectableItemFromIndex(g-b);if(SC.none(g)||(g<0)){g=0
}a=g;d=null}var c=g;f=SC.IndexSet.create(g,a+1-g);this.scrollToContentIndex(c);this.select(f);
this._selectionAnchor=d;return this},selectNextItem:function(h,i){if(SC.none(i)){i=1
}if(SC.none(h)){h=false}var b=this.get("selection"),g=this.get("content");if(b){b=b.indexSetForSource(g)
}var a=b?b.get("min"):-1,d=b?b.get("max")-1:-1,e=this._selectionAnchor,c=this.get("length");
if(SC.none(e)){e=a}if(h){if(a<e){a=a+i}else{d=this._findNextSelectableItemFromIndex(d+i,d)
}if(d>=c){d=c-1}if(a>d){a=d}}else{d=this._findNextSelectableItemFromIndex(d+i,d);
if(d>=c){d=c-1}a=d;e=null}var f=d;b=SC.IndexSet.create(a,d-a+1);this.scrollToContentIndex(f);
this.select(b);this._selectionAnchor=e;return this},deleteSelection:function(){if(!this.get("canDeleteContent")){return NO
}var d=this.get("selection"),c=this.get("content"),a=this.get("selectionDelegate"),b=d&&c?d.indexSetForSource(c):null;
if(!c||!b||b.get("length")===0){return NO}b=a.collectionViewShouldDeleteIndexes(this,b);
if(!b||b.get("length")===0){return NO}a.collectionViewDeleteContent(this,this.get("content"),b);
this.selectPreviousItem(false,1);return YES},scrollToContentIndex:function(b){var a=this.itemViewForContentIndex(b);
if(a){this.scrollToItemView(a)}return this},scrollToItemView:function(a){if(a){a.scrollToVisible()
}return this},keyDown:function(a){var b=this.interpretKeyEvents(a);return !b?NO:b
},keyUp:function(){return true},insertText:function(b,a){if(b===" "){var c=this.get("selection");
if(c&&c.get("length")>0){this.invokeLater(this._cv_action,0,null,a)}return YES}else{return NO
}},selectAll:function(a){var b=this.get("content"),c=b?SC.IndexSet.create(0,b.get("length")):null;
this.select(c,NO);return YES},deleteBackward:function(a){return this.deleteSelection()
},deleteForward:function(a){return this.deleteSelection()},moveDown:function(b,a){this.selectNextItem(false,this.get("itemsPerRow")||1);
this._cv_performSelectAction(null,a,this.ACTION_DELAY);return true},moveUp:function(b,a){this.selectPreviousItem(false,this.get("itemsPerRow")||1);
this._cv_performSelectAction(null,a,this.ACTION_DELAY);return true},moveLeft:function(f,l){if((this.get("itemsPerRow")||1)>1){this.selectPreviousItem(false,1);
this._cv_performSelectAction(null,l,this.ACTION_DELAY)}else{var c=this.get("selection"),j=this.get("content"),h=c?c.indexSetForSource(j):null;
if(h){var m=undefined,g=false,i=undefined;if(h.get("length")===1){i=h.get("firstObject");
m=this.get("contentDelegate");var b=m.contentIndexDisclosureState(this,j,i);if(b!==SC.BRANCH_OPEN){g=true
}}if(g){var a=m.contentIndexOutlineLevel(this,j,i)-1;if(a>=0){var e=-1;while(e<0){var d=this._findPreviousSelectableItemFromIndex(i-1);
if(d<0){return false}i=d;var k=m.contentIndexOutlineLevel(this,j,i);if(k===a){e=d
}}if(e!==-1){this.select(i)}}}else{this.collapse(h)}}}return true},moveRight:function(c,a){if((this.get("itemsPerRow")||1)>1){this.selectNextItem(false,1);
this._cv_performSelectAction(null,a,this.ACTION_DELAY)}else{var e=this.get("selection"),d=this.get("content"),b=e?e.indexSetForSource(d):null;
if(b){this.expand(b)}}return true},moveDownAndModifySelection:function(b,a){this.selectNextItem(true,this.get("itemsPerRow")||1);
this._cv_performSelectAction(null,a,this.ACTION_DELAY);return true},moveUpAndModifySelection:function(b,a){this.selectPreviousItem(true,this.get("itemsPerRow")||1);
this._cv_performSelectAction(null,a,this.ACTION_DELAY);return true},moveLeftAndModifySelection:function(b,a){if((this.get("itemsPerRow")||1)>1){this.selectPreviousItem(true,1);
this._cv_performSelectAction(null,a,this.ACTION_DELAY)}return true},moveRightAndModifySelection:function(b,a){if((this.get("itemsPerRow")||1)>1){this.selectNextItem(true,1);
this._cv_performSelectAction(null,a,this.ACTION_DELAY)}return true},insertNewline:function(d,c){var b=this.get("isEditable")&&this.get("canEditContent"),g,f,h,a,e;
if(b){g=this.get("selection");f=this.get("content");if(g&&g.get("length")===1){h=g.indexSetForSource(f);
a=h?h.get("min"):-1;b=a>=0}}if(b){e=this.itemViewForContentIndex(a);b=e&&SC.typeOf(e.beginEditing)===SC.T_FUNCTION
}if(b){this.scrollToContentIndex(a);e=this.itemViewForContentIndex(a);e.beginEditing()
}else{this.invokeLater(this._cv_action,0,e,null)}return YES},mouseDown:function(h){if(this.get("useToggleSelection")){return true
}var g=this.itemViewForEvent(h),f=this.get("content"),e=g?g.get("contentIndex"):-1,c,d;
c=this.mouseDownInfo={event:h,itemView:g,contentIndex:e,at:Date.now()};this.becomeFirstResponder();
if(!g){if(this.get("allowDeselectAll")){this.select(null,false)}return YES}var b=this.get("selection"),a,i;
if(b){b=b.indexSetForSource(f)}a=b?b.contains(e):NO;c.modifierKeyPressed=i=h.ctrlKey||h.metaKey;
if(i&&a){c.shouldDeselect=e>=0}else{if(h.shiftKey&&b&&b.get("length")>0){b=this._findSelectionExtendedByShift(b,e);
d=this._selectionAnchor;this.select(b);this._selectionAnchor=d}else{if(!i&&a){c.shouldReselect=e>=0
}else{if(this.get("selectOnMouseDown")){this.select(e,i)}else{c.shouldSelect=e>=0
}}}}c.previousContentIndex=e;return YES},mouseUp:function(h){var i=this.itemViewForEvent(h),d=this.mouseDownInfo,e,c,a,b,g,f,j;
if(this.get("useToggleSelection")){if(!i){return}c=this.get("selection");e=(i)?i.get("contentIndex"):-1;
a=c&&c.include(e);if(a){this.deselect(e)}else{this.select(e,YES)}}else{if(d){j=d.contentIndex;
e=(i)?i.get("contentIndex"):-1;if(d.shouldSelect){this.select(j,d.modifierKeyPressed)
}if(d.shouldDeselect){this.deselect(j)}if(d.shouldReselect){b=this.get("isEditable")&&this.get("canEditContent");
if(b){c=this.get("selection");b=c&&(c.get("length")===1)}if(b){g=this.itemViewForContentIndex(j);
b=g&&(!g.contentHitTest||g.contentHitTest(h));b=(b&&g.beginEditing)?g.beginEditing():NO
}if(!b){if(this._cv_reselectTimer){this._cv_reselectTimer.invalidate()}this._cv_reselectTimer=this.invokeLater(this.select,300,j,false)
}}this._cleanupMouseDown()}}this._cv_performSelectAction(i,h,0,h.clickCount);return NO
},_cleanupMouseDown:function(){var b=this.mouseDownInfo,a;if(b){for(a in b){if(!b.hasOwnProperty(a)){continue
}delete b[a]}}this.mouseDownInfo=null},mouseMoved:function(c){var a=this.itemViewForEvent(c),b=this._lastHoveredItem;
if(a!==b){if(b&&b.mouseOut){b.mouseOut(c)}if(a&&a.mouseOver){a.mouseOver(c)}}this._lastHoveredItem=a;
if(a&&a.mouseMoved){a.mouseMoved(c)}return YES},mouseOut:function(b){var a=this._lastHoveredItem;
this._lastHoveredItem=null;if(a&&a.mouseOut){a.mouseOut(b)}return YES},_findSelectionExtendedByShift:function(e,h){if(!e||e.get("length")===0){return SC.IndexSet.create(h)
}var d=this.get("content"),g=d.get("length")-1,c=e.get("min"),a=e.get("max")-1,f=this.mouseDownInfo,b=this._selectionAnchor;
if(SC.none(b)){b=-1}if(h<c){c=h;if(b<0){this._selectionAnchor=b=a}}else{if(h>a){a=h;
if(b<0){this._selectionAnchor=b=c}}else{if(h>=c&&h<=a){if(b<0){this._selectionAnchor=b=c
}if(h===b){c=a=h}else{if(h>b){c=b;a=h}else{if(h<b){c=h;a=b}}}}}}return SC.IndexSet.create(c,a-c+1)
},reorderDataType:function(){return"SC.CollectionView.Reorder.%@".fmt(SC.guidFor(this))
}.property().cacheable(),dragContent:null,proposedInsertionIndex:null,proposedDropOperation:null,mouseDragged:function(f){var a=this.get("selectionDelegate"),e=this.get("content"),g=this.get("selection"),h=this.mouseDownInfo,d,c,b;
if(!h||h.contentIndex<0){return YES}if((Date.now()-h.at)<123){return YES}if(a.collectionViewShouldBeginDrag(this)){if(!this.get("selectOnMouseDown")){d=SC.IndexSet.create(h.contentIndex)
}else{d=g?g.indexSetForSource(e):null}if(!d){return YES}d={content:e,indexes:d};this.set("dragContent",d);
c=this.get("dragDataTypes");if(c&&c.get("length")>0){b=a.collectionViewDragViewFor(this,d.indexes);
if(!b){b=this._cv_dragViewFor(d.indexes)}b.createLayer();SC.Drag.start({event:h.event,source:this,dragView:b,ghost:NO,ghostActsLikeCursor:a.ghostActsLikeCursor,slideBack:YES,dataSource:this});
this._cleanupMouseDown();this._lastInsertionIndex=null}else{this.set("dragContent",null)
}return YES}},_cv_dragViewFor:function(d){var b=this.get("nowShowing").without(d);
b=this.get("nowShowing").without(b);var c=this.get("layer").cloneNode(false);var a=SC.View.create({layer:c,parentView:this});
SC.$(c).css("backgroundColor","transparent").css("border","none").css("top",0).css("left",0);
b.forEach(function(g){var h=this.itemViewForContentIndex(g),e,f;if(h){e=h.get("isSelected");
h.set("isSelected",NO);h.updateLayerIfNeeded();f=h.get("layer");if(f){f=f.cloneNode(true)
}h.set("isSelected",e);h.updateLayerIfNeeded()}if(f){c.appendChild(f)}f=null},this);
c=null;return a},dragDataTypes:function(){var a=this.get("selectionDelegate"),b=a.collectionViewDragDataTypes(this),c;
if(this.get("canReorderContent")){b=b?b.copy():[];c=this.get("reorderDataType");if(b.indexOf(c)<0){b.push(c)
}}return b?b:[]}.property(),dragDataForType:function(c,b){if(this.get("canReorderContent")){if(b===this.get("reorderDataType")){return this.get("dragContent")
}}var a=this.get("selectionDelegate");return a.collectionViewDragDataForType(this,c,b)
},computeDragOperations:function(c,b){var d=SC.DRAG_NONE,a=this.get("selectionDelegate");
if(this.get("canReorderContent")){if(c.get("dataTypes").indexOf(this.get("reorderDataType"))>=0){d=SC.DRAG_REORDER
}}d=a.collectionViewComputeDragOperations(this,c,d);if(d&SC.DRAG_REORDER){d=SC.DRAG_MOVE
}return d},_computeDropOperationState:function(b,k,d){var f=this.convertFrameFromView(b.get("location"),null),j=SC.DROP_BEFORE,l=this.get("selectionDelegate"),c=this.get("canReorderContent"),m,g,a,h,e;
var i=this.insertionIndexForLocation(f,SC.DROP_ON);if(SC.typeOf(i)===SC.T_ARRAY){j=i[1];
i=i[0]}if(j===SC.DROP_ON){this.set("proposedInsertionIndex",i);this.set("proposedDropOperation",j);
d=l.collectionViewValidateDragOperation(this,b,d,i,j);i=this.get("proposedInsertionIndex");
j=this.get("proposedDropOperation");this._dropInsertionIndex=this._dropOperation=null;
if(d!==SC.DRAG_NONE){return[i,j,d]}else{j=SC.DROP_BEFORE;i=this.insertionIndexForLocation(f,SC.DROP_BEFORE);
if(SC.typeOf(i)===SC.T_ARRAY){j=i[1];i=i[0]}}}if((i>=0)&&c&&(j!==SC.DROP_ON)){m=b.dataForType(this.get("reorderDataType"));
if(m){g=this.get("content");if(j===SC.DROP_BEFORE){a=m.indexes.contains(i-1);h=m.indexes.contains(i)
}else{a=m.indexes.contains(i);h=m.indexes.contains(i-1)}if(a&&h){if(SC.none(this._lastInsertionIndex)){if(j===SC.DROP_BEFORE){while((i>=0)&&m.indexes.contains(i)){i--
}}else{e=g?g.get("length"):0;while((i<e)&&m.indexes.contains(i)){i++}}}else{i=this._lastInsertionIndex
}}if(i>=0){d=SC.DRAG_REORDER}}}this.set("proposedInsertionIndex",i);this.set("proposedDropOperation",j);
d=l.collectionViewValidateDragOperation(this,b,d,i,j);i=this.get("proposedInsertionIndex");
j=this.get("proposedDropOperation");this._dropInsertionIndex=this._dropOperation=null;
return[i,j,d]},dragUpdated:function(f,b){var h=f.get("allowedDragOperations"),g=this._computeDropOperationState(f,b,h),a=g[0],c=g[1],e=g[2];
if(e!==SC.DRAG_NONE){if((this._lastInsertionIndex!==a)||(this._lastDropOperation!==c)){var d=this.itemViewForContentIndex(a);
this.showInsertionPoint(d,c)}this._lastInsertionIndex=a;this._lastDropOperation=c
}else{this.hideInsertionPoint();this._lastInsertionIndex=this._lastDropOperation=null
}return(e&SC.DRAG_REORDER)?SC.DRAG_MOVE:e},dragExited:function(){this.hideInsertionPoint();
this._lastInsertionIndex=this._lastDropOperation=null},acceptDragOperation:function(a,b){return YES
},performDragOperation:function(e,f){var a=this._computeDropOperationState(e,null,f),j=a[0],i=a[1],g=a[2],k=this.get("selectionDelegate"),c,l,d,h,b;
if(g&SC.DRAG_REORDER){f=(f&SC.DRAG_MOVE)?SC.DRAG_REORDER:SC.DRAG_NONE}else{f=f&g}if(f===SC.DRAG_NONE){return f
}c=k.collectionViewPerformDragOperation(this,e,f,j,i);if((c===SC.DRAG_NONE)&&(f&SC.DRAG_REORDER)){d=e.dataForType(this.get("reorderDataType"));
if(!d){return SC.DRAG_NONE}h=this.get("content");h.beginPropertyChanges();l=[];b=0;
d.indexes.forEach(function(m){l.push(h.objectAt(m-b));h.removeAt(m-b);b++;if(m<j){j--
}if((i===SC.DROP_AFTER)&&(m===j)){j--}},this);h.replace(j,0,l);this.select(SC.IndexSet.create(j,l.length));
h.endPropertyChanges();f=SC.DRAG_MOVE}return f},collectionViewShouldBeginDrag:function(a){return this.get("canReorderContent")
},insertionIndexForLocation:function(a,b){return -1},_cv_isVisibleInWindowDidChange:function(){if(this.get("isVisibleInWindow")){if(this._invalidIndexes){this.invokeOnce(this.reloadIfNeeded)
}if(this._invalidSelection){this.invokeOnce(this.reloadSelectionIndexesIfNeeded)}}}.observes("isVisibleInWindow"),collectionViewShouldSelectItem:function(a,b){return this.get("isSelectable")
},_TMP_DIFF1:SC.IndexSet.create(),_TMP_DIFF2:SC.IndexSet.create(),_cv_nowShowingDidChange:function(){var b=this.get("nowShowing"),a=this._sccv_lastNowShowing,d,e,c;
if(a!==b){if(a&&b){e=this._TMP_DIFF1.add(a).remove(b);c=this._TMP_DIFF2.add(b).remove(a);
d=e.add(c)}else{d=a||b}}if(d&&d.get("length")>0){this._sccv_lastNowShowing=b?b.frozenCopy():null;
this.updateContentRangeObserver();this.reload(d)}if(e){e.clear()}if(c){c.clear()}}.observes("nowShowing"),init:function(){arguments.callee.base.apply(this,arguments);
if(this.get("canReorderContent")){this._cv_canReorderContentDidChange()}this._sccv_lastNowShowing=this.get("nowShowing").clone();
if(this.content){this._cv_contentDidChange()}if(this.selection){this._cv_selectionDidChange()
}},_cv_canReorderContentDidChange:function(){if(this.get("canReorderContent")){if(!this.get("isDropTarget")){this.set("isDropTarget",YES)
}SC.Drag.addDropTarget(this)}}.observes("canReorderContent"),_cv_performSelectAction:function(b,d,c,a){var e;
if(c===undefined){c=0}if(a===undefined){a=1}if((a>1)||this.get("actOnSelect")){if(this._cv_reselectTimer){this._cv_reselectTimer.invalidate()
}e=this.get("selection");e=e?e.toArray():[];if(this._cv_actionTimer){this._cv_actionTimer.invalidate()
}this._cv_actionTimer=this.invokeLater(this._cv_action,c,b,d,e)}},_cv_action:function(b,a,c){var d=this.get("action");
var e=this.get("target")||null;this._cv_actionTimer=null;if(d){if(SC.typeOf(d)===SC.T_FUNCTION){return this.action(b,a)
}var f=this.get("pane");if(f){f.rootResponder.sendAction(d,e,this,f,c)}}else{if(!b){return
}else{if(SC.typeOf(b._action)==SC.T_FUNCTION){return b._action(a)}else{if(SC.typeOf(b.action)==SC.T_FUNCTION){return b.action(a)
}}}}}});SC.DisclosureView=SC.ButtonView.extend({classNames:["sc-disclosure-view"],theme:"disclosure",buttonBehavior:SC.TOGGLE_BEHAVIOR,toggleOnValue:YES,toggleOffValue:NO,valueBindingDefault:SC.Binding.bool(),render:function(a,b){a.push('<img src="',SC.BLANK_IMAGE_URL,'" class="button" alt="" />');
if(this.get("needsEllipsis")){a.push('<label class="ellipsis">',this.get("displayTitle"),"</label>")
}else{a.push("<label>",this.get("displayTitle"),"</label>")}},keyDown:function(a){if(a.which===37||a.which===38){this.set("value",this.get("toggleOffValue"));
return YES}if(a.which===39||a.which===40){this.set("value",this.get("toggleOnValue"));
return YES}arguments.callee.base.apply(this,arguments)}});SC.FormView=SC.View.extend({content:null,contentBindingDefault:SC.Binding.Single,isDirty:false,isCommitting:true,isEnabled:true,passThroughToContent:false,isValid:function(){return this.get("errors").length==0
}.property("errors"),canCommit:function(){return this.get("isValid")&&this.get("isEnabled")
}.property("isValid","isEnabled"),generalErrors:null,errors:function(){if(!this._fields){return[]
}if(!this._errors){var a=this;this._errors=[];this.get("fieldKeys").each(function(b){var c=a.get(b);
if($type(c)==T_ERROR){a._errors.push(c)}})}return this._errors.concat(this.get("generalErrors")||[])
}.property("generalErrors"),fieldKeys:function(){if(!this._fieldKeys&&this._fields){var b=[];
for(var a in this._fields){if(!this._fields.hasOwnProperty(a)){continue}b.push(a)
}this._fieldKeys=b}return this._fieldKeys}.property(),validate:function(){if(!this._fields){return true
}for(var a in this._fields){if(this._fields.hasOwnProperty(a)){var b=this._fields[a];
if(b.validateSubmit){b.validateSubmit()}}}return this.get("isValid")},commit:function(){if(!this.validate()){return false
}var a=true;var b=this.get("content");if(!b||!this._fields){return}var c=this.get("isEnabled");
this.beginPropertyChanges();this.set("isEnabled",false);this.set("isCommitting",true);
this.endPropertyChanges();a=this.get("passThroughToContent")?this._commitChanges():this._copyContentAndCommitChanges();
this.beginPropertyChanges();this.set("isCommitting",false);this.set("isEnabled",c);
this.endPropertyChanges();return a},_copyContentAndCommitChanges:function(){var a=true;
var c=this.get("content");if(!c||!this._fields){return false}try{c.beginPropertyChanges();
for(var b in this._fields){if(b.match(/Button$/)){continue}if(this._fields.hasOwnProperty(b)){var f=this.get(b);
c.set(b,f)}}c.endPropertyChanges();a=this._commitChanges();this.set("isDirty",!a)
}catch(d){console.log("commit() exception: "+d);a=false}return a},_commitChanges:function(){var a=this.get("content");
var b=false;if(a&&a.commit){b=a.commit(this)}else{if(a&&a.commitChanges){b=a.commitChanges()
}}return b},reset:function(){if(!this._fields){return}var b=this.get("content");if(b&&b.discardChanges){b.discardChanges()
}this.beginPropertyChanges();for(var a in this._fields){if(this._fields.hasOwnProperty(a)){var c=(b)?b.get(a):null;
this.set(a,c)}}this.set("isDirty",false);this.endPropertyChanges()},rebuildFields:function(){this.beginPropertyChanges();
if(this._fields){for(var a in this._fields){if(this._fields.hasOwnProperty(a)){this.removeField(a)
}}}this._fields={};this._buttons={};this._values={};this._rebuildFieldsForNode(this,true);
this.endPropertyChanges()},addField:function(a,c){if(this[a]!==undefined){throw"FormView cannot add the field '%@' because that property already exists.  Try using another name.".fmt(a)
}var b=this;if(a=="submitButton"&&(c.action==SC.ButtonView.prototype.action)){c.action=function(){b.commit()
}}if(a=="resetButton"&&(c.action==SC.ButtonView.prototype.action)){c.action=function(){b.reset()
}}this._fields[a]=c;if(a.substr(-6,6)=="Button"){this._buttons[a]=c}this.propertyWillChange(a);
this.setValueForField(a,c.get("value"));this.propertyDidChange(a,this.getValueForField(a));
c.addObserver("value",this._fieldValueObserver_b());c.set("ownerForm",this);this.propertyWillChange("fieldKeys");
this._fieldKeys=null;this.propertyDidChange("fieldKeys",null)},removeField:function(a){var b=this._fields[a];
if(b){b.removeObserver("value",this._fieldValueObserver_b());b.set("ownerForm",null)
}this.propertyWillChange(a);delete this._fields[a];delete this._values[a];delete this._buttons[a];
this.propertyDidChange(a,null);this.propertyWillChange("fieldKeys");this._fieldKeys=null;
this.propertyDidChange("fieldKeys",null)},getField:function(a){return this._fields[a]
},keyDown:function(a){return this.interpretKeyEvents(a)},keyUp:function(){},insertNewline:function(c,a){var b=this._findDefaultButton(this);
if(!b&&this._fields&&this._fields.submitButton){b=this._fields.submitButton}if(b&&b.triggerAction){b.triggerAction(a)
}return true},_findDefaultButton:function(a){if(a.triggerAction&&a.get("isDefault")){return a
}a=a.firstChild;while(a){var b=this._findDefaultButton(a);if(b){return b}a=a.nextSibling
}return null},unknownProperty:function(c,e){var f=(this._fields)?this._fields[c]:null;
if(e!==undefined){if(f){var b=this.getValueForField(c);this.setValueForField(c,e);
f.set("value",e);var d=$type(b)==T_ERROR;var a=$type(e)==T_ERROR;if(d!=a){this.propertyWillChange("errors");
this._errors=null;this.propertyDidChange("errors",null)}}else{this[c]=e}}else{if(f){if(this.getValueForField(c)===undefined){this.setValueForField(c,f.get("value"))
}return this.getValueForField(c)}}return e},getValueForField:function(a){if(this.get("passThroughToContent")){var b=this.get("content");
return(b&&b.get)?b.get(a):undefined}else{return this._values[a]}},setValueForField:function(a,c){if(this.get("passThroughToContent")){var b=this.get("content");
if(b&&b.get&&b.set&&(b.get(a)!==c)){b.set(a,c)}}else{this._values[a]=c}return c},init:function(){arguments.callee.base.apply(this,arguments);
if(this.rootElement&&this.rootElement.tagName.toLowerCase()=="form"){this.rootElement.onsubmit=function(){return false
}}this.rebuildFields()},_rebuildFieldsForNode:function(c,b){if(c.fieldKey){this.addField(c.fieldKey,c)
}if((b!=true)&&(c instanceof SC.FormView)){return}var a=(c.childNodesForFormField)?c.childNodesForFormField():c.get("childNodes");
var d=a.length;while(--d>=0){c=a[d];this._rebuildFieldsForNode(c,false)}},_fieldValueObserver:function(f,c,e){if(!(c=f.fieldKey)){return
}var b=this.getValueForField(c);if(b==e){return}this.beginPropertyChanges();this.propertyWillChange(c);
this.setValueForField(c,e);this.propertyDidChange(c,e);var d=$type(b)==T_ERROR;var a=$type(e)==T_ERROR;
if(d!=a){this.propertyWillChange("errors");this._errors=null;this.propertyDidChange("errors",null)
}if(!this.get("isDirty")){this.set("isDirty",true)}this.endPropertyChanges()},_fieldValueObserver_b:function(){return this._bound_fieldValueObserver=(this._bound_fieldValueObserver||this._fieldValueObserver.bind(this))
},_contentPropertyObserver:function(c,b,d){if(!this._fields||!c){return}var a=this._fields;
if(a[b]&&c.didChangeFor(this,b)){this.set(b,d)}else{if(b=="*"){for(var b in a){if(a.hasOwnProperty(b)&&c.didChangeFor(this,b)){this.set(b,c.get(b))
}}}}},_contentPropertyObserver_b:function(){return this._bound_contentPropertyObserver=(this._bound_contentPropertyObserver||this._contentPropertyObserver.bind(this))
},_isEnabledObserver:function(){var a=this._fields;if(!a){return}var b=this.get("isEnabled");
var d=this.get("canCommit");for(var c in a){if(a.hasOwnProperty(c)){var e=a[c];if(e.set){if(c=="submitButton"){e.set("isEnabled",d)
}else{e.set("isEnabled",b)}}}}}.observes("isEnabled"),_contentObserver:function(){var b=this.get("content");
if(b==this._content){return}var a=this._contentPropertyObserver_b();if(this._content){this._content.removeObserver("*",a)
}this._content=b;if(!b){return}b.addObserver("*",a);this.reset()}.observes("content"),_canCommitObserver:function(){var c=this._buttons;
var b=this.get("canCommit");if(c&&c.submitButton){var a=c.submitButton;if(a.set){a.set("isEnabled",b)
}}}.observes("canCommit")});sc_require("views/collection");sc_require("mixins/collection_row_delegate");
SC.ListView=SC.CollectionView.extend(SC.CollectionRowDelegate,{classNames:["sc-list-view"],acceptsFirstResponder:YES,rowDelegate:function(){var a=this.delegate,b=this.get("content");
return this.delegateFor("isCollectionRowDelegate",a,b)}.property("delegate","content").cacheable(),_sclv_rowDelegateDidChange:function(){var d=this._sclv_rowDelegate,b=this.get("rowDelegate"),c=this._sclv_rowHeightDidChange,a=this._sclv_customRowHeightIndexesDidChange;
if(d===b){return this}this._sclv_rowDelegate=b;if(d){d.removeObserver("rowHeight",this,c);
d.removeObserver("customRowHeightIndexes",this,a)}if(!b){throw"Internal Inconsistancy: ListView must always have CollectionRowDelegate"
}b.addObserver("rowHeight",this,c);b.addObserver("customRowHeightIndexes",this,a);
this._sclv_rowHeightDidChange()._sclv_customRowHeightIndexesDidChange();return this
}.observes("rowDelegate"),_sclv_rowHeightDidChange:function(){var b=this.get("rowDelegate"),a=b.get("rowHeight"),c;
if(a===this._sclv_rowHeight){return this}this._sclv_rowHeight=a;c=SC.IndexSet.create(0,this.get("length"));
this.rowHeightDidChangeForIndexes(c);return this},_sclv_customRowHeightIndexesDidChange:function(){var a=this.get("rowDelegate"),b=a.get("customRowHeightIndexes"),d=this._sclv_customRowHeightIndexes,c=this._sclv_customRowHeightIndexesContentDidChange;
if((b===d)||(d&&d.isEqual(b))){return this}if(d&&this._sclv_isObservingCustomRowHeightIndexes){d.removeObserver("[]",this,c)
}if(this._sclv_isObservingCustomRowHeightIndexes=b&&!b.get("isFrozen")){b.addObserver("[]",this,c)
}this._sclv_customRowHeightIndexesContentDidChange();return this},_sclv_customRowHeightIndexesContentDidChange:function(){var a=this.get("rowDelegate"),b=a.get("customRowHeightIndexes"),c=this._sclv_customRowHeightIndexes,d;
if(b&&c){d=b.copy().add(c)}else{d=b||c}this._sclv_customRowHeightIndexes=b?b.frozenCopy():null;
this.rowHeightDidChangeForIndexes(d);return this},rowOffsetForContentIndex:function(g){if(g===0){return 0
}var i=this.get("rowDelegate"),a=i.get("rowHeight"),e,c,b,h,f,d;e=g*a;if(this.get("rowSpacing")){e+=g*this.get("rowSpacing")
}if(i.customRowHeightIndexes&&(c=i.get("customRowHeightIndexes"))){b=this._sclv_offsetCache;
if(!b){b=this._sclv_offsetCache=[];h=f=0;c.forEach(function(j){h+=this.rowHeightForContentIndex(j)-a;
b[j+1]=h;f=j},this);this._sclv_max=f+1}h=b[g];if(h===undefined){h=b[g]=b[g-1];if(h===undefined){f=this._sclv_max;
if(g<f){f=c.indexBefore(g)+1}h=b[g]=b[f]||0}}e+=h}return e},rowHeightForContentIndex:function(a){var b=this.get("rowDelegate"),e,c,f,d;
if(b.customRowHeightIndexes&&(d=b.get("customRowHeightIndexes"))){c=this._sclv_heightCache;
if(!c){c=this._sclv_heightCache=[];f=this.get("content");d.forEach(function(g){c[g]=b.contentIndexRowHeight(this,f,g)
},this)}e=c[a];if(e===undefined){e=b.get("rowHeight")}}else{e=b.get("rowHeight")}return e
},rowHeightDidChangeForIndexes:function(b){var a=this.get("length");this._sclv_heightCache=this._sclv_offsetCache=null;
if(b&&b.isIndexSet){b=b.get("min")}this.reload(SC.IndexSet.create(b,a-b));return this
},computeLayout:function(){var a=this._sclv_layout;if(!a){a=this._sclv_layout={}}a.minHeight=this.rowOffsetForContentIndex(this.get("length"))+4;
return a},layoutForContentIndex:function(a){return{top:this.rowOffsetForContentIndex(a),height:this.rowHeightForContentIndex(a),left:0,right:0}
},contentIndexesInRect:function(h){var a=this.get("rowDelegate").get("rowHeight"),g=SC.minY(h),b=SC.maxY(h),i=h.height||0,f=this.get("length"),e,c,d;
c=(g-(g%a))/a;e=this.rowOffsetForContentIndex(c);while(c>0&&e>=g){c--;e-=this.rowHeightForContentIndex(c)
}e+=this.rowHeightForContentIndex(c);while(c<f&&e<g){e+=this.rowHeightForContentIndex(c);
c++}if(c<0){c=0}if(c>=f){c=f}d=c+((i-(i%a))/a);if(d>f){d=f}e=this.rowOffsetForContentIndex(d);
while(d>=c&&e>=b){d--;e-=this.rowHeightForContentIndex(d)}e+=this.rowHeightForContentIndex(d);
while(d<f&&e<=b){e+=this.rowHeightForContentIndex(d);d++}d++;if(d<c){d=c}if(d>f){d=f
}return SC.IndexSet.create(c,d-c)},insertionPointView:SC.View.extend({classNames:"sc-list-insertion-point",render:function(a,b){if(b){a.push('<div class="anchor"></div>')
}}}),showInsertionPoint:function(c,f){var b=this._insertionPointView;if(!b){b=this._insertionPointView=this.get("insertionPointView").create()
}var d=SC.clone(c.get("layout")),e=c.get("outlineLevel"),a=c.get("outlineIndent")||0;
if(SC.none(e)){e=-1}if(f&SC.DROP_ON){this.hideInsertionPoint();c.set("isSelected",YES);
this._lastDropOnView=c}else{if(this._lastDropOnView){this._lastDropOnView.set("isSelected",NO);
this._lastDropOnView=null}if(f&SC.DROP_AFTER){d.top+=d.height}d.height=2;d.right=0;
d.left=((e+1)*a)+12;delete d.width;b.set("layout",d);this.appendChild(b)}},hideInsertionPoint:function(){if(this._lastDropOnView){this._lastDropOnView.set("isSelected",NO);
this._lastDropOnView=null}var a=this._insertionPointView;if(a){a.removeFromParent().destroy()
}this._insertionPointView=null},insertionIndexForLocation:function(e,i){var d=this.contentIndexesInRect(e),f=d.get("min"),g=this.get("length"),a,j,k,c,m,b,l,h;
if(SC.none(f)||f<0){if((g===0)||(e.y<=this.rowOffsetForContentIndex(0))){f=0}else{if(e.y>=this.rowOffsetForContentIndex(g)){f=g
}}}a=this.rowOffsetForContentIndex(f);j=a+this.rowHeightForContentIndex(f);if(i==SC.DROP_ON){if(this.get("isEditable")){k=Math.min(Math.floor((j-a)*0.2),5)
}else{k=0}if(e.y>=(a+k)||e.y<=(j+k)){return[f,SC.DROP_ON]}}if((f<g)&&(e.y>=j-10)){f++
}if(f>0){h=this.itemViewForContentIndex(f);c=h?h.get("outlineLevel"):0;m=(h?h.get("outlineIndent"):0)||0;
m*=c;h=this.itemViewForContentIndex(f);l=(h?h.get("outlineIndent"):0)||0;b=h?h.get("outlineLevel"):0;
l*=b;if((c!==b)&&(m!==l)){if(((l>m)&&(e.x>=l))||((l<m)&&(e.x<=m))){f--;i=SC.DROP_AFTER
}}}return[f,i]},init:function(){arguments.callee.base.apply(this,arguments);this._sclv_rowDelegateDidChange()
}});require("views/list");SC.GridView=SC.ListView.extend({classNames:["sc-grid-view"],layout:{left:0,right:0,top:0,bottom:0},rowHeight:48,columnWidth:64,exampleView:SC.LabelView,insertionOrientation:SC.HORIZONTAL_ORIENTATION,displayProperties:"itemsPerRow".w(),itemsPerRow:function(){var b=this.get("frame");
var a=this.get("columnWidth")||0;return(a<=0)?1:Math.floor(b.width/a)}.property("frame","columnWidth").cacheable(),itemsPerRowDidChange:function(){this.set("isDirty",YES)
}.observes("itemsPerRow"),contentIndexesInRect:function(e){var d=this.get("rowHeight")||48;
var b=this.get("itemsPerRow");var c=Math.floor(SC.minY(e)/d)*b;var a=Math.ceil(SC.maxY(e)/d)*b;
return SC.IndexSet.create(c,a-c)},layoutForContentIndex:function(g){var d=this.get("rowHeight")||48;
var a=this.get("frame").width;var b=this.get("itemsPerRow");var e=Math.floor(a/b);
var f=Math.floor(g/b);var c=g-(b*f);return{left:c*e,top:f*d,height:d,width:e}},computeLayout:function(){var e=this.get("content");
var d=(e)?e.get("length"):0;var c=this.get("rowHeight")||48;var a=this.get("itemsPerRow");
var f=Math.ceil(d/a);var b=this._cachedLayoutHash;if(!b){b=this._cachedLayoutHash={}
}b.minHeight=f*c;return b},insertionPointClass:SC.View.extend({classNames:["grid-insertion-point"],render:function(a,b){if(b){a.push('<span class="anchor"></span>')
}}}),showInsertionPoint:function(c,e){if(!c){return}if(e===SC.DROP_ON){if(c!==this._dropOnInsertionPoint){this.hideInsertionPoint();
c.addClassName("drop-target");this._dropOnInsertionPoint=c}}else{if(this._dropOnInsertionPoint){this._dropOnInsertionPoint.removeClassName("drop-target");
this._dropOnInsertionPoint=null}if(!this._insertionPointView){this._insertionPointView=this.insertionPointClass.create()
}var b=this._insertionPointView;var a=c.get("frame");var d={height:a.height-6,x:a.x,y:a.y+6,width:0};
if(!SC.rectsEqual(b.get("frame"),d)){b.set("frame",d)}if(b.parentNode!=c.parentNode){c.parentNode.appendChild(b)
}}},hideInsertionPoint:function(){var a=this._insertionPointView;if(a){a.removeFromParent()
}if(this._dropOnInsertionPoint){this._dropOnInsertionPoint.removeClassName("drop-target");
this._dropOnInsertionPoint=null}},insertionIndexForLocation:function(d,j){var e=this.get("frame");
var g=this.get("scrollFrame");var k=this.get("itemsPerRow");var a=Math.floor(e.width/k);
var m=Math.floor((d.y-e.y-g.y)/this.get("rowHeight"));var i=SC.DROP_BEFORE;var c=(d.x-e.x-g.x);
var b=Math.floor(c/a);var l=(c/a)-b;if(j===SC.DROP_ON){if(l>0.8){b++}if((l>=0.2)&&(l<=0.8)){i=SC.DROP_ON
}}else{if(l>0.45){b++}}var h=(m*k)+b;return[h,i]}});SC.NATURAL_SCROLLER_THICKNESS=16;
SC.ScrollerView=SC.View.extend({classNames:["sc-scroller-view"],scrollerThickness:SC.NATURAL_SCROLLER_THICKNESS,value:function(a,c){if(c!==undefined){if(c>=0){this._value=c
}}else{var b=this._value||0;return Math.min(b,this.get("maximum"))}}.property("maximum").cacheable(),maximum:0,isEnabled:YES,layoutDirection:SC.LAYOUT_VERTICAL,ownerScrollValueKey:function(){var a=null;
switch(this.get("layoutDirection")){case SC.LAYOUT_VERTICAL:a="verticalScrollOffset";
break;case SC.LAYOUT_HORIZONTAL:a="horizontalScrollOffset";break;default:a=null}return a
}.property("layoutDirection").cacheable(),displayProperties:"maximum isEnabled layoutDirection".w(),render:function(b,c){var a=this.get("maximum");
switch(this.get("layoutDirection")){case SC.LAYOUT_VERTICAL:b.addClass("sc-vertical");
if(c){b.push('<div class="sc-inner" style="height: %@px;">&nbsp;</div>'.fmt(a))}else{this.$("div")[0].style.height=a+"px"
}break;case SC.LAYOUT_HORIZONTAL:b.addClass("sc-horizontal");if(c){b.push('<div class="sc-inner" style="width: %@px;">&nbsp;</div>'.fmt(a))
}else{this.$("div")[0].style.width=a+"px"}break;default:throw"You must set a layoutDirection for your scroller class."
}b.setClass("disabled",!this.get("isEnabled"))},didCreateLayer:function(){var c=this._sc_scroller_scrollDidChange;
SC.Event.add(this.$(),"scroll",this,c);var b=this.get("value");var a=this.get("layer");
switch(this.get("layoutDirection")){case SC.LAYOUT_VERTICAL:a.scrollTop=b;break;case SC.LAYOUT_HORIZONTAL:a.scrollLeft=b;
break}},willDestroyLayer:function(){var a=this._sc_scroller_scrollDidChange;SC.Event.remove(this.$(),"scroll",this,a)
},_sc_scroller_armScrollTimer:function(){if(!this._sc_scrollTimer){SC.RunLoop.begin();
var a=this._sc_scroller_scrollDidChange;this._sc_scrollTimer=this.invokeLater(a,50);
SC.RunLoop.end()}},_sc_scroller_scrollDidChange:function(){var b=Date.now(),d=this._sc_lastScroll;
if(d&&(b-d)<50){return this._sc_scroller_armScrollTimer()}this._sc_scrollTimer=null;
this._sc_lastScroll=b;SC.RunLoop.begin();if(!this.get("isEnabled")){return}var c=this.get("layer"),a=0;
switch(this.get("layoutDirection")){case SC.LAYOUT_VERTICAL:this._sc_scrollValue=a=c.scrollTop;
break;case SC.LAYOUT_HORIZONTAL:this._sc_scrollValue=a=c.scrollLeft;break}this.set("value",a);
SC.RunLoop.end()},_sc_scroller_valueDidChange:function(){var a=this.get("value");
if(a!==this._sc_scrollValue){var b=this.get("layer");if(b){switch(this.get("layoutDirection")){case SC.LAYOUT_VERTICAL:b.scrollTop=a;
break;case SC.LAYOUT_HORIZONTAL:b.scrollLeft=a;break}}}}.observes("value")});sc_require("views/scroller");
sc_require("mixins/border");SC.ScrollView=SC.View.extend(SC.Border,{classNames:["sc-scroll-view"],isScrollable:YES,contentView:null,horizontalScrollOffset:0,verticalScrollOffset:0,maximumHorizontalScrollOffset:function(){if(!this.get("canScrollHorizontal")){return 0
}var b=this.get("contentView");var a=b?b.get("frame").width:0;var c=this.get("containerView").get("frame").width;
return Math.max(0,a-c)}.property(),maximumVerticalScrollOffset:function(){if(!this.get("canScrollVertical")){return 0
}var a=this.get("contentView");var b=(a&&a.get("frame"))?a.get("frame").height:0;
var c=this.get("containerView").get("frame").height;return Math.max(0,b-c)}.property(),verticalLineScroll:20,horizontalLineScroll:20,verticalPageScroll:function(){return this.get("frame").height
}.property("frame"),horizontalPageScroll:function(){return this.get("frame").width
}.property("frame"),hasHorizontalScroller:YES,horizontalScrollerView:SC.ScrollerView,isHorizontalScrollerVisible:YES,canScrollHorizontal:function(){return !!(this.get("hasHorizontalScroller")&&this.get("horizontalScrollerView")&&this.get("isHorizontalScrollerVisible"))
}.property("isHorizontalScrollerVisible").cacheable(),autohidesHorizontalScroller:YES,hasVerticalScroller:YES,verticalScrollerView:SC.ScrollerView,isVerticalScrollerVisible:YES,canScrollVertical:function(){return !!(this.get("hasVerticalScroller")&&this.get("verticalScrollerView")&&this.get("isVerticalScrollerVisible"))
}.property("isVerticalScrollerVisible").cacheable(),autohidesVerticalScroller:YES,verticalScrollerBottom:0,containerView:SC.ContainerView,scrollTo:function(a,b){if(b===undefined&&SC.typeOf(a)===SC.T_HASH){b=a.y;
a=a.x}if(!SC.none(a)){a=Math.max(0,Math.min(this.get("maximumHorizontalScrollOffset"),a));
this.set("horizontalScrollOffset",a)}if(!SC.none(b)){b=Math.max(0,Math.min(this.get("maximumVerticalScrollOffset"),b));
this.set("verticalScrollOffset",b)}return this},scrollBy:function(a,b){if(b===undefined&&SC.typeOf(a)===SC.T_HASH){b=a.y;
a=a.x}a=(a)?this.get("horizontalScrollOffset")+a:null;b=(b)?this.get("verticalScrollOffset")+b:null;
return this.scrollTo(a,b)},scrollToVisible:function(b){if(arguments.length===0){return arguments.callee.base.apply(this,arguments)
}var d=this.get("contentView");if(!d){return NO}var a=b.get("frame");if(!a){return NO
}a=d.convertFrameFromView(a,b.get("parentView"));var c=SC.cloneRect(this.get("containerView").get("frame"));
c.x=this.get("horizontalScrollOffset");c.y=this.get("verticalScrollOffset");var f=c.x,e=c.y;
c.y-=Math.max(0,SC.minY(c)-SC.minY(a));c.x-=Math.max(0,SC.minX(c)-SC.minX(a));c.y+=Math.max(0,SC.maxY(a)-SC.maxY(c));
c.x+=Math.max(0,SC.maxX(a)-SC.maxX(c));if((f!==c.x)||(e!==c.y)){this.scrollTo(c.x,c.y);
return YES}else{return NO}},scrollDownLine:function(a){if(a===undefined){a=1}return this.scrollBy(null,this.get("verticalLineScroll")*a)
},scrollUpLine:function(a){if(a===undefined){a=1}return this.scrollBy(null,0-this.get("verticalLineScroll")*a)
},scrollRightLine:function(a){if(a===undefined){a=1}return this.scrollTo(this.get("horizontalLineScroll")*a,null)
},scrollLeftLine:function(a){if(a===undefined){a=1}return this.scrollTo(0-this.get("horizontalLineScroll")*a,null)
},scrollDownPage:function(a){if(a===undefined){a=1}return this.scrollBy(null,this.get("verticalPageScroll")*a)
},scrollUpPage:function(a){if(a===undefined){a=1}return this.scrollBy(null,0-(this.get("verticalPageScroll")*a))
},scrollRightPage:function(a){if(a===undefined){a=1}return this.scrollBy(this.get("horizontalPageScroll")*a,null)
},scrollLeftPage:function(a){if(a===undefined){a=1}return this.scrollBy(0-(this.get("horizontalPageScroll")*a),null)
},tile:function(){var a=this.get("hasHorizontalScroller")?this.get("horizontalScrollerView"):null;
var d=a&&this.get("isHorizontalScrollerVisible");var f=this.get("hasVerticalScroller")?this.get("verticalScrollerView"):null;
var c=f&&this.get("isVerticalScrollerVisible");var b=this.get("containerView");var i={left:0,top:0};
var h;var e=((d)?a.get("scrollerThickness"):0);var g=(c)?f.get("scrollerThickness"):0;
if(d){a.set("layout",{left:0,bottom:0,right:g-1,height:e});i.bottom=e-1}else{i.bottom=0
}if(a){a.set("isVisible",d)}if(c){e=e+this.get("verticalScrollerBottom");f.set("layout",{top:0,bottom:e,right:0,width:g});
i.right=g-1}else{i.right=0}if(f){f.set("isVisible",c)}b.set("layout",i)},scrollerVisibilityDidChange:function(){this.tile()
}.observes("isVerticalScrollerVisible","isHorizontalScrollerVisible"),_scroll_wheelDeltaX:0,_scroll_wheelDeltaY:0,mouseWheel:function(a){this._scroll_wheelDeltaX+=a.wheelDeltaX;
this._scroll_wheelDeltaY+=a.wheelDeltaY;this.invokeLater(this._scroll_mouseWheel,10);
return YES},_scroll_mouseWheel:function(){this.scrollBy(this._scroll_wheelDeltaX,this._scroll_wheelDeltaY);
this._scroll_wheelDeltaX=this._scroll_wheelDeltaY=0},createChildViews:function(){var b=[],a;
if(SC.none(a=this.containerView)){a=SC.ContainerView}b.push(this.containerView=this.createChildView(a,{contentView:this.contentView}));
this.contentView=this.containerView.get("contentView");if(a=this.horizontalScrollerView){if(this.get("hasHorizontalScroller")){a=this.horizontalScrollerView=this.createChildView(a,{layoutDirection:SC.LAYOUT_HORIZONTAL,valueBinding:"*owner.horizontalScrollOffset"});
b.push(a)}else{this.horizontalScrollerView=null}}if(a=this.verticalScrollerView){if(this.get("hasVerticalScroller")){a=this.verticalScrollerView=this.createChildView(a,{layoutDirection:SC.LAYOUT_VERTICAL,valueBinding:"*owner.verticalScrollOffset"});
b.push(a)}else{this.verticalScrollerView=null}}this.childViews=b;this.contentViewDidChange();
this.tile()},init:function(){arguments.callee.base.apply(this,arguments);this._scroll_contentView=this.get("contentView");
var a=this._scroll_contentView;if(a){a.addObserver("frame",this,this.contentViewFrameDidChange)
}if(this.get("isVisibleInWindow")){this._scsv_registerAutoscroll()}},_scsv_registerAutoscroll:function(){if(this.get("isVisibleInWindow")){SC.Drag.addScrollableView(this)
}else{SC.Drag.removeScrollableView(this)}}.observes("isVisibleInWindow"),contentViewDidChange:function(){var c=this.get("contentView"),a=this._scroll_contentView;
var b=this.contentViewFrameDidChange;if(c!==a){if(a){a.removeObserver("frame",this,b)
}this._scroll_contentView=c;if(c){c.addObserver("frame",this,b)}this.containerView.set("content",c);
this.contentViewFrameDidChange()}}.observes("contentView"),oldMaxHOffset:0,oldMaxVOffset:0,contentViewFrameDidChange:function(){var h=this.get("contentView"),g=(h)?h.get("frame"):null,a=(g)?g.width:0,j=(g)?g.height:0,e=this.get("frame");
this._scroll_contentWidth=a;this._scroll_contentHeight=j;if(this.get("hasHorizontalScroller")&&(h=this.get("horizontalScrollerView"))){a-=1;
if(this.get("autohidesHorizontalScroller")){this.set("isHorizontalScrollerVisible",a>e.width)
}h.setIfChanged("maximum",a)}if(this.get("hasVerticalScroller")&&(h=this.get("verticalScrollerView"))){j-=1;
if(this.get("autohidesVerticalScroller")){this.set("isVerticalScrollerVisible",j>e.height)
}j-=this.get("verticalScrollerBottom");h.setIfChanged("maximum",j)}var c=this.get("maximumVerticalScrollOffset")&&this.get("hasVerticalScroller")&&this.get("maximumVerticalScrollOffset")<=this.get("verticalScrollOffset");
var b=this.get("maximumHorizontalScrollOffset")&&this.get("hasHorizontalScroller")&&this.get("maximumHorizontalScrollOffset")<=this.get("horizontalScrollOffset");
var i=this.get("maximumHorizontalScrollOffset");var d=this.get("maximumVerticalScrollOffset");
if(c||b){this.forceDimensionsRecalculation(b,c)}},_scroll_horizontalScrollOffsetDidChange:function(){var b=this.get("horizontalScrollOffset");
b=Math.max(0,Math.min(this.get("maximumHorizontalScrollOffset"),b));var a=this.get("contentView");
if(a){a.adjust("left",0-b)}}.observes("horizontalScrollOffset"),_scroll_verticalScrollOffsetDidChange:function(){var c=this.get("verticalScrollOffset");
c=Math.max(0,Math.min(this.get("maximumVerticalScrollOffset"),c));var b=this.get("contentView");
var a=this.get("containerView");if(b){b.adjust("top",0-c)}}.observes("verticalScrollOffset"),forceDimensionsRecalculation:function(a,b){var d=this.get("horizontalScrollOffset");
var c=this.get("verticalScrollOffset");this.scrollTo(0,0);if(a&&b){this.scrollTo(this.get("maximumHorizontalScrollOffset")-0.001,this.get("maximumVerticalScrollOffset")-0.001)
}if(a&&!b){this.scrollTo(this.get("maximumHorizontalScrollOffset")-0.001,c)}if(!a&&b){this.scrollTo(d,this.get("maximumVerticalScrollOffset")-0.001)
}}});sc_require("views/scroll");SC.MenuScrollerView=SC.ScrollerView.extend({classNames:["sc-menu-scroller-view"],scrollDown:NO,value:function(a,c){if(c!==undefined){this._value=c
}else{var b=this._value||0;return Math.min(b,this.get("maximum"))}}.property("maximum").cacheable(),maximum:0,isEnabled:YES,layoutDirection:SC.LAYOUT_VERTICAL,verticalLineScroll:20,ownerScrollValueKey:function(){return"verticalScrollOffset"
}.property("layoutDirection").cacheable(),render:function(a,b){a.addClass("sc-vertical");
if(b){if(this.get("scrollDown")){a.push('<span class="arrowDown">&nbsp;</span>')}else{a.push('<span class="arrowUp">&nbsp;</span>')
}}a.setClass("disabled",!this.get("isEnabled"))},didCreateLayer:function(){var c,b,a;
c=this._sc_scroller_scrollDidChange;SC.Event.add(this.$(),"scroll",this,c);b=this.get("value");
a=this.get("layer");a.scrollTop=b},willDestroyLayer:function(){var a=this._sc_scroller_scrollDidChange;
SC.Event.remove(this.$(),"scroll",this,a)},mouseEntered:function(a){this.set("isMouseOver",YES);
this._invokeScrollOnMouseOver()},mouseExited:function(a){this.set("isMouseOver",NO)
},_sc_scroller_valueDidChange:function(){}.observes("value"),_sc_scroller_armScrollTimer:function(){if(!this._sc_scrollTimer){SC.RunLoop.begin();
var a=this._sc_scroller_scrollDidChange;this._sc_scrollTimer=this.invokeLater(a,50);
SC.RunLoop.end()}},_sc_scroller_scrollDidChange:function(){var b=Date.now(),d=this._sc_lastScroll,c=this.get("layer"),a=0;
if(d&&(b-d)<50){return this._sc_scroller_armScrollTimer()}this._sc_scrollTimer=null;
this._sc_lastScroll=b;SC.RunLoop.begin();if(!this.get("isEnabled")){return}this._sc_scrollValue=a=c.scrollTop;
this.set("value",a);SC.RunLoop.end()},_scrollMenu:function(){var b=this.get("value"),a;
if(this.get("scrollDown")){a=b+this.verticalLineScroll;if(a<=this.get("maximum")){this.set("value",a)
}}else{a=b-this.verticalLineScroll;if(a>=0){this.set("value",a)}else{if(b<=this.verticalLineScroll&&b>0){this.set("value",0)
}}}return YES},_invokeScrollOnMouseOver:function(){this._scrollMenu();if(this.get("isMouseOver")){this.invokeLater(this._invokeScrollOnMouseOver,50)
}}});SC.MenuScrollView=SC.ScrollView.extend({classNames:["sc-menu-scroll-view"],maximumHorizontalScrollOffset:function(){}.property(),hasHorizontalScroller:NO,horizontalScrollerView:SC.MenuScrollerView,isHorizontalScrollerVisible:NO,canScrollHorizontal:function(){return false
}.property("isHorizontalScrollerVisible").cacheable(),autohidesHorizontalScroller:NO,hasVerticalScroller:YES,verticalScrollerView:SC.MenuScrollerView,verticalScrollerView2:SC.MenuScrollerView,isVerticalScrollerVisible:YES,canScrollVertical:function(){return YES
}.property("isVerticalScrollerVisible").cacheable(),autohidesVerticalScroller:YES,verticalScrollerBottom:0,containerView:SC.ContainerView,tile:function(){var m,o,e,d,c,q,h;
m=this.get("hasVerticalScroller");o=m?this.get("verticalScrollerView"):null;e=m?this.get("verticalScrollerView2"):null;
d=o&&this.get("isVerticalScrollerVisible");c=this.get("containerView");q={left:0,top:0};
if(d){h=0;var l=this.get("contentView"),k,i=(l)?l.get("frame"):null,p=(i)?i.height:0,b=this.containerView.$()[0],r=this.get("verticalScrollOffset"),j={height:0,top:0,right:0,left:0},a={height:this.verticalLineScroll,top:0,right:0,left:0},g={height:this.verticalLineScroll,bottom:0,right:0,left:0},n={height:0,bottom:0,right:0,left:0};
if(b){h=b.offsetHeight}if(r===0){q.top=0;q.bottom=this.verticalLineScroll;o.set("layout",j);
e.set("layout",g)}else{if(r>=(p-h-this.verticalLineScroll)){q.top=this.verticalLineScroll;
q.bottom=0;o.set("layout",a);e.set("layout",n)}else{q.top=this.verticalLineScroll;
q.bottom=this.verticalLineScroll;o.set("layout",a);e.set("layout",g)}}}if(o){o.set("isVisible",d);
e.set("isVisible",d)}c.set("layout",q)},scrollerVisibilityDidChange:function(){this.tile()
}.observes("isVerticalScrollerVisible","isHorizontalScrollerVisible","verticalScrollOffset"),createChildViews:function(){var c=[],b,a;
if(SC.none(b=this.containerView)){b=SC.ContainerView}c.push(this.containerView=this.createChildView(b,{contentView:this.contentView}));
this.contentView=this.containerView.get("contentView");if((b=this.verticalScrollerView)&&(a=this.verticalScrollerView2)){if(this.get("hasVerticalScroller")){b=this.verticalScrollerView=this.createChildView(b,{layout:{top:0,left:0,right:0,height:this.verticalLineScroll},valueBinding:"*owner.verticalScrollOffset"});
c.push(b);a=this.verticalScrollerView2=this.createChildView(a,{scrollDown:YES,layout:{bottom:0,left:0,right:0,height:this.verticalLineScroll},valueBinding:"*owner.verticalScrollOffset"});
c.push(a)}else{this.verticalScrollerView=null;this.verticalScrollerView2=null}}this.childViews=c;
this.contentViewFrameDidChange();this.tile()},init:function(){arguments.callee.base.apply(this,arguments);
this._scroll_contentView=this.get("contentView");var a=this._scroll_contentView;if(a){a.addObserver("frame",this,this.contentViewFrameDidChange)
}if(this.get("isVisibleInWindow")){this._scsv_registerAutoscroll()}},_scsv_registerAutoscroll:function(){if(this.get("isVisibleInWindow")){SC.Drag.addScrollableView(this)
}else{SC.Drag.removeScrollableView(this)}}.observes("isVisibleInWindow"),contentViewDidChange:function(){var c=this.get("contentView"),a=this._scroll_contentView,b=this.contentViewFrameDidChange;
if(c!==a){if(a){a.removeObserver("frame",this,b)}this._scroll_contentView=c;if(c){c.addObserver("frame",this,b)
}this.containerView.set("content",c);this.contentViewFrameDidChange()}}.observes("contentView"),contentViewFrameDidChange:function(){var c=this.get("contentView"),b,h=(c)?c.get("frame"):null,e=(h)?h.width:0,a=(h)?h.height:0,i=this.get("frame"),d,g;
this._scroll_contentWidth=e;this._scroll_contentHeight=a;if(this.get("hasVerticalScroller")&&(c=this.get("verticalScrollerView"))&&(b=this.get("verticalScrollerView2"))){a-=1;
if(this.get("autohidesVerticalScroller")){this.set("isVerticalScrollerVisible",a>i.height)
}a-=this.get("verticalScrollerBottom");d=0;g=this.containerView.$()[0];if(g){d=g.offsetHeight
}a=a-d;c.setIfChanged("maximum",a);b.setIfChanged("maximum",a)}},_scroll_horizontalScrollOffsetDidChange:function(){}.observes("horizontalScrollOffset"),_scroll_verticalScrollOffsetDidChange:function(){var b=this.get("verticalScrollOffset");
var a=this.get("contentView");if(a){a.adjust("top",0-b)}}.observes("verticalScrollOffset")});
sc_require("views/button");SC.PopupButtonView=SC.ButtonView.extend({keyEquivalent:null,classNames:["sc-popup-button"],preferMatrix:null,acceptsFirstResponder:YES,isSelected:NO,performKeyEquivalent:function(b,a){if(!this.get("isEnabled")){return NO
}var c=this.get("menu");return(!!c&&c.performKeyEquivalent(b,a))},menu:null,isSelectedBinding:"*menu.isVisibleInWindow",action:function(a){var b=this.get("menu");
if(!b){return NO}b.popup(this,this.preferMatrix);return YES},mouseDown:function(a){if(!this.get("isEnabled")){return YES
}this.set("isActive",YES);this._isMouseDown=YES;this._action();return YES}});SC.ProgressView=SC.View.extend(SC.Control,{value:0.5,valueBindingDefault:SC.Binding.single().notEmpty(),minimum:0,minimumBindingDefault:SC.Binding.single().notEmpty(),contentMinimumKey:null,maximum:1,maximumBindingDefault:SC.Binding.single().notEmpty(),offsetRange:24,contentMaximumKey:null,isIndeterminate:NO,isIndeterminateBindingDefault:SC.Binding.bool(),isRunning:NO,isRunningBindingDefault:SC.Binding.bool(),animatedBackgroundMatrix:[],contentIsIndeterminateKey:null,classNames:"sc-progress-view",_backgroundOffset:0,_currentBackground:1,_nextBackground:1,init:function(){arguments.callee.base.apply(this,arguments);
this.animateProgressBar()},animateProgressBar:function(){if(this.get("isRunning")&&this.get("isVisibleInWindow")){this._animateProgressBar(500)
}}.observes("isRunning","isVisibleInWindow"),_animateProgressBar:function(a){if(a===0){a=1000/30
}if(this.get("isRunning")&&this.get("isVisibleInWindow")){this.displayDidChange();
this.invokeLater(this._animateProgressBar,a,0)}},displayProperties:"value minimum maximum isIndeterminate".w(),render:function(c,b){var n,e;
var f=this.get("isIndeterminate");var m=this.get("isRunning");var k=this.get("isEnabled");
var l=this.get("offsetRange");var g=(f&&m)?(Math.floor(Date.now()/75)%l-l):0;var j;
if(!k){j="0%"}else{if(f){j="120%"}else{var h=this.get("minimum")||0;var d=this.get("maximum")||1;
j=this.get("value")||0;j=(j-h)/(d-h);if(j>1){j=1}if(isNaN(j)){j=0}if(j<h){j=0}if(j>d){j=1
}j=(j*100)+"%"}}var a={"sc-indeterminate":f,"sc-empty":(j<=0),"sc-complete":(j>=100)};
if(b){var i=this._createClassNameString(a);c.push('<div class="sc-inner ',i,'" style="width: ',j,";left: ",g,'">');
c.push('<div class="sc-inner-head"></div><div class="sc-inner-tail"></div></div><div class="sc-outer-head"></div><div class="sc-outer-tail"></div>')
}else{c.setClass(a);n=this.$(".sc-inner");e=this.get("animatedBackgroundMatrix");
n.css("width",j).css("left",g);if(e.length===3){n.css("backgroundPosition","0px -"+(e[0]+e[1]*this._currentBackground)+"px");
if(this._currentBackground===e[2]-1||this._currentBackground===0){this._nextBackground*=-1
}this._currentBackground+=this._nextBackground}}},contentPropertyDidChange:function(c,a){var b=this.get("content");
this.beginPropertyChanges().updatePropertyFromContent("value",a,"contentValueKey",b).updatePropertyFromContent("minimum",a,"contentMinimumKey",b).updatePropertyFromContent("maximum",a,"contentMaximumKey",b).updatePropertyFromContent("isIndeterminate",a,"contentIsIndeterminateKey",b).endPropertyChanges()
},_createClassNameString:function(c){var b=[],a;for(a in c){if(!c.hasOwnProperty(a)){continue
}if(c[a]){b.push(a)}}return b.join(" ")}});SC.RadioView=SC.FieldView.extend({classNames:["sc-radio-view"],value:null,layoutDirection:SC.LAYOUT_VERTICAL,escapeHTML:YES,items:[],itemTitleKey:null,itemValueKey:null,itemIsEnabledKey:null,itemIconKey:null,displayItems:function(){var e=this.get("items"),b=this.get("localize"),o=this.get("itemTitleKey"),n=this.get("itemValueKey"),c=this.get("itemIsEnabledKey"),l=this.get("itemIconKey");
var d=[],g=(e)?e.get("length"):0;var m,h,k,j,a,i,f;for(j=0;j<g;j++){m=e.objectAt(j);
if(SC.typeOf(m)===SC.T_ARRAY){h=m[0];k=m[1]}else{if(m){if(o){h=m.get?m.get(o):m[o]
}else{h=(m.toString)?m.toString():null}if(n){k=m.get?m.get(n):m[n]}else{k=m}if(c){i=m.get?m.get(c):m[c]
}else{i=YES}if(l){f=m.get?m.get(l):m[l]}else{f=null}}else{h=k=f=null;i=NO}}if(b){h=h.loc()
}d.push([h,k,i,f])}return d}.property("items","itemTitleKey","itemValueKey","itemIsEnabledKey","localize","itemIconKey").cacheable(),itemsDidChange:function(){if(this._items){this._items.removeObserver("[]",this,this.itemContentDidChange)
}this._items=this.get("items");if(this._items){this._items.addObserver("[]",this,this.itemContentDidChange)
}this.itemContentDidChange()}.observes("items"),itemContentDidChange:function(){this.notifyPropertyChange("displayItems")
},$input:function(){return this.$("input")},displayProperties:["value","displayItems"],render:function(f,a){var p,o,l,d,q,e,k,h,g,m,c,j=this.get("displayItems"),n=this.get("value"),i=SC.isArray(n);
f.addClass(this.get("layoutDirection"));if(i&&n.length<=0){n=n[0];i=NO}if(a){d=SC.guidFor(this);
q=j.length;for(o=0;o<q;o++){p=j[o];l=p[3];if(l){e=(l.indexOf("/")>=0)?l:"/static/sproutcore/foundation/en/bcfdf5ca1125ccb312799938d8f09a81676f5db0/blank.gif";
k=(e===l)?"":l;l='<img src="%@" class="icon %@" alt="" />'.fmt(e,k)}else{l=""}c=this._getSelectionState(p,n,i,false);
h=(!p[2])||(!this.get("isEnabled"))?'disabled="disabled" ':"";g=this.escapeHTML?SC.RenderContext.escapeHTML(p[0]):p[0];
var b="/static/sproutcore/foundation/en/bcfdf5ca1125ccb312799938d8f09a81676f5db0/blank.gif";
f.push('<label class="sc-radio-button ',c,'">');f.push('<input type="radio" value="',o,'" name="',d,'" ',h,"/>");
f.push('<span class="button"></span>');f.push('<span class="sc-button-label">',l,g,"</span></label>")
}this._field_setFieldValue(this.get("value"))}else{this.$input().forEach(function(r){r=this.$(r);
o=parseInt(r.val(),0);p=(o>=0)?j[o]:null;r.attr("disabled",(!p[2])?"disabled":null);
m=this._getSelectionState(p,n,i,true);r.parent().setClass(m);r=o=m=null},this)}},_getSelectionState:function(d,f,a,b){var e,h,c;
if(d){e=(a)?(f.indexOf(d[1])>=0):(f===d[1])}else{e=NO}h={sel:(e&&!a),mixed:(e&&a),disabled:(!d[2])};
if(b){return h}else{var g=[];for(c in h){if(!h.hasOwnProperty(c)){continue}if(h[c]){g.push(c)
}}return g.join(" ")}},getFieldValue:function(){var b=this.$input().filter(function(){return this.checked
}).val();var a=this.get("displayItems");b=a[parseInt(b,0)];return b?b[1]:this._mixedValue
},setFieldValue:function(c){if(SC.isArray(c)){if(c.get("length")>1){this._mixedValue=c;
c=undefined}else{c=c.objectAt(0)}}var b,a;if(c===undefined){a=-1}else{b=this.get("displayItems");
a=b.indexOf(b.find(function(d){return d[1]===c}))}this.$input().forEach(function(d){d=SC.$(d);
d.attr("checked",parseInt(d.val(),0)===a);d=null});return this},didCreateLayer:function(){this.setFieldValue(this.get("fieldValue"));
var b=this.$input();for(var a=0;a<b.length;a++){SC.Event.add(b[a],"click",this,this._field_fieldValueDidChange)
}},willDestroyLayer:function(){var b=this.$input();for(var a=0;a<b.length;a++){SC.Event.remove(this.$input()[a],"click",this,this._field_fieldValueDidChange)
}},mouseDown:function(a){return YES}});SC.SceneView=SC.ContainerView.extend({scenes:["master","detail"],nowShowing:null,transitionDuration:200,_state:"NO_VIEW",replaceContent:function(a){if(a&&this._state===this.READY){this.animateScene(a)
}else{this.replaceScene(a)}return this},replaceScene:function(c){var d=this._targetView,e=this.STANDARD_LAYOUT,b=this.get("scenes"),a=b?b.indexOf(this.get("nowShowing")):-1;
this._targetView=c;this._targetIndex=a;if(this._timer){this._timer.invalidate()}this._leftView=this._rightView=this._start=this._end=null;
this._timer=null;this.removeAllChildren();if(d){d.set("layout",e)}if(c){c.set("layout",e)
}if(c){this.appendChild(c)}this._state=c?this.READY:this.NO_VIEW},animateScene:function(b){var c=this._targetView,f=this._targetIndex,a=this.get("scenes"),e=a?a.indexOf(this.get("nowShowing")):-1,d;
if(f<0||e<0||f===e){return this.replaceScene(b)}this._targetView=b;this._targetIndex=e;
if(e>f){this._leftView=c;this._rightView=b;this._target=-1}else{this._leftView=b;
this._rightView=c;this._target=1}this.removeAllChildren();if(c){this.appendChild(c)
}if(b){this.appendChild(b)}this._start=Date.now();this._end=this._start+this.get("transitionDuration");
this._state=this.ANIMATING;this.tick()},tick:function(){this._timer=null;var a=Date.now(),d=(a-this._start)/(this._end-this._start),g=this._target,f=this._leftView,b=this._rightView,c,e;
if(d<0){d=0}if(!this.get("isVisibleInWindow")||(d>=1)){return this.replaceScene(this._targetView)
}c=SC.clone(this.get("frame"));e=Math.floor(c.width*d);if(g>0){c.left=0-(c.width-e);
f.set("layout",c);c=SC.clone(c);c.left=e;b.set("layout",c)}else{c.left=0-e;f.set("layout",c);
c=SC.clone(c);c.left=c.width-e;b.set("layout",c)}this._timer=this.invokeLater(this.tick,20);
return this},NO_VIEW:"NO_VIEW",ANIMATING:"ANIMATING",READY:"READY",STANDARD_LAYOUT:{top:0,left:0,bottom:0,right:0}});
SC.SegmentedView=SC.View.extend(SC.Control,{classNames:["sc-segmented-view"],value:null,isEnabled:YES,allowsEmptySelection:NO,allowsMultipleSelection:NO,localize:YES,layoutDirection:SC.LAYOUT_HORIZONTAL,items:[],itemTitleKey:null,itemValueKey:null,itemIsEnabledKey:null,itemIconKey:null,itemWidthKey:null,itemActionKey:null,itemTargetKey:null,itemKeyEquivalentKey:null,itemKeys:"itemTitleKey itemValueKey itemIsEnabledKey itemIconKey itemWidthKey itemToolTipKey".w(),displayItems:function(){var f=this.get("items"),c=this.get("localize");
var k=null,d,i;var e=[],g=f.get("length"),h,j;var b=SC._segmented_fetchKeys;var a=SC._segmented_fetchItem;
for(h=0;h<g;h++){j=f.objectAt(h);if(SC.none(j)){continue}d=SC.typeOf(j);if(d===SC.T_STRING){i=[j.humanize().titleize(),j,YES,null,null,null,h]
}else{if(d!==SC.T_ARRAY){if(k===null){k=this.itemKeys.map(b,this)}i=k.map(a,j);i[i.length]=h;
if(!k[0]&&j.toString){i[0]=j.toString()}if(!k[1]){i[1]=j}if(!k[2]){i[2]=YES}}}if(c&&i[0]){i[0]=i[0].loc()
}if(c&&i[5]&&SC.typeOf(i[5])===SC.T_STRING){i[5]=i[5].loc()}e[e.length]=i}return e
}.property("items","itemTitleKey","itemValueKey","itemIsEnabledKey","localize","itemIconKey","itemWidthKey","itemToolTipKey").cacheable(),itemsDidChange:function(){if(this._items){this._items.removeObserver("[]",this,this.itemContentDidChange)
}this._items=this.get("items");if(this._items){this._items.addObserver("[]",this,this.itemContentDidChange)
}this.itemContentDidChange()}.observes("items"),itemContentDidChange:function(){this.notifyPropertyChange("displayItems")
},init:function(){arguments.callee.base.apply(this,arguments);this.itemsDidChange()
},displayProperties:["displayItems","value","activeIndex"],render:function(b,a){var f=this.get("displayItems");
var i=this._seg_displayItems;if(a||(f!==i)){this._seg_displayItems=f;this.renderDisplayItems(b,f);
b.addStyle("text-align","center")}else{var k=this.get("activeIndex");var h=this.get("value");
var c=SC.isArray(h);if(c&&h.get("length")===1){h=h.objectAt(0);c=NO}var g={};var d=f.length,e=this.$("a.sc-segment"),j;
while(--d>=0){j=f[d];g.sel=c?(h.indexOf(j[1])>=0):(j[1]===h);g.active=(k===d);SC.$(e.get(d)).setClass(g)
}g=f=h=f=null}},renderDisplayItems:function(d,k){var n=this.get("value"),g=SC.isArray(n),p=this.get("activeIndex"),h=k.length,m,l,b,j,e,o,a,c,f;
for(f=0;f<h;f++){e=d.begin("a").attr("href","javascript:;");o=k[f];m=o[0];l=o[3];
a=o[5];e.addStyle("display","inline-block");e.addClass("sc-segment");if(!o[2]){e.addClass("disabled")
}if(f===0){e.addClass("sc-first-segment")}if(f===(h-1)){e.addClass("sc-last-segment")
}if(f!==0&&f!==(h-1)){e.addClass("sc-middle-segment")}if(g?(n.indexOf(o[1])>=0):(o[1]===n)){e.addClass("sel")
}if(p===f){e.addClass("active")}if(o[4]){c=o[4];e.addStyle("width",c+"px")}if(a){e.attr("title",a)
}if(l){b=(l.indexOf("/")>=0)?l:"/static/sproutcore/foundation/en/bcfdf5ca1125ccb312799938d8f09a81676f5db0/blank.gif";
j=(b===l)?"":l;l='<img src="'+b+'" alt="" class="icon '+j+'" />'}else{l=""}e.push('<span class="sc-button-inner"><label class="sc-button-label">');
e.push(l+m);e.push("</label></span>");e.end()}},displayItemIndexForEvent:function(b){var d=SC.$(b.target);
if(!d||d===document){return -1}var a=this.$(),c=null;while(!c&&(d.length>0)&&(d.get(0)!==a.get(0))){if(d.hasClass("sc-segment")&&d.attr("tagName")==="A"){c=d
}else{d=d.parent()}}d=a=null;return(c)?this.$("a.sc-segment").index(c):-1},keyDown:function(d){var f,g,e,a,h,c;
if(d.which===9){var b=d.shiftKey?this.get("previousValidKeyView"):this.get("nextValidKeyView");
b.becomeFirstResponder();return YES}if(!this.get("allowsMultipleSelection")&&!this.get("allowsEmptySelection")){e=this.get("displayItems");
a=e.length;h=this.get("value");c=SC.isArray(h);if(d.which===39||d.which===40){for(f=0;
f<a-1;f++){g=e[f];if(c?(h.indexOf(g[1])>=0):(g[1]===h)){this.triggerItemAtIndex(f+1)
}}return YES}else{if(d.which===37||d.which===38){for(f=1;f<a;f++){g=e[f];if(c?(h.indexOf(g[1])>=0):(g[1]===h)){this.triggerItemAtIndex(f-1)
}}return YES}}}return YES},mouseDown:function(b){if(!this.get("isEnabled")){return YES
}var a=this.displayItemIndexForEvent(b);if(a>=0){this._isMouseDown=YES;this.set("activeIndex",a)
}return YES},mouseUp:function(b){var a=this.displayItemIndexForEvent(b);if(this._isMouseDown&&(a>=0)){this.triggerItemAtIndex(a)
}this._isMouseDown=NO;this.set("activeIndex",-1);return YES},mouseMoved:function(b){var a=this.displayItemIndexForEvent(b);
if(this._isMouseDown){this.set("activeIndex",a)}return YES},mouseOver:function(b){var a=this.displayItemIndexForEvent(b);
if(this._isMouseDown){this.set("activeIndex",a)}return YES},mouseOut:function(a){if(this._isMouseDown){this.set("activeIndex",-1)
}return YES},triggerItemAtIndex:function(k){var i=this.get("displayItems"),l=i.objectAt(k),b,j,c,g,f;
if(!l[2]){return this}g=this.get("allowsEmptySelection");f=this.get("allowsMultipleSelection");
b=l[1];j=c=this.get("value");if(!SC.isArray(j)){j=[j]}if(!f){if(g&&(j.get("length")===1)&&(j.objectAt(0)===b)){j=[]
}else{j=[b]}}else{if(j.indexOf(b)>=0){if(j.get("length")>1||(j.objectAt(0)!==b)||g){j=j.without(b)
}}else{j=j.concat([b])}}switch(j.get("length")){case 0:j=null;break;case 1:j=j.objectAt(0);
break;default:break}var m=this.get("itemActionKey");var a=this.get("itemTargetKey");
var e,h=null;var d=this.getPath("pane.rootResponder");if(m&&(l=this.get("items").objectAt(l[6]))){e=l.get?l.get(m):l[m];
if(a){h=l.get?l.get(a):l[a]}if(d){d.sendAction(e,h,this,this.get("pane"))}}if(!e&&c!==undefined){this.set("value",j)
}e=this.get("action");if(e&&d){d.sendAction(e,this.get("target"),this,this.get("pane"))
}},acceptsFirstResponder:function(){return this.get("isEnabled")}.property("isEnabled"),willBecomeKeyResponderFrom:function(a){if(!this._isFocused){this._isFocused=YES;
this.becomeFirstResponder();if(this.get("isVisibleInWindow")){this.$()[0].focus()
}}},willLoseKeyResponderTo:function(a){if(this._isFocused){this._isFocused=NO}}});
SC._segmented_fetchKeys=function(a){return this.get(a)};SC._segmented_fetchItem=function(a){if(!a){return null
}return this.get?this.get(a):this[a]};SC.SelectFieldView=SC.FieldView.extend({tagName:"select",classNames:["sc-select-field-view"],objects:[],objectsBindingDefault:SC.Binding.multiple(),nameKey:null,sortKey:null,valueKey:null,emptyName:null,localize:false,cpDidChange:YES,disableSort:NO,validateMenuItem:function(b,a){return true
},sortObjects:function(b){if(!this.get("disableSort")){var a=this.get("sortKey")||this.get("nameKey");
b=b.sort(function(d,c){if(a){d=d.get?d.get(a):d[a];c=c.get?c.get(a):c[a]}return(d<c)?-1:((d>c)?1:0)
})}return b},render:function(c,a){if(this.get("cpDidChange")){this.set("cpDidChange",NO);
var f=this.get("nameKey");var j=this.get("valueKey");var i=this.get("objects");var b=this.get("value");
var d,g;var h=this.get("localize");if(!j&&b){b=SC.guidFor(b)}if((b===null)||(b==="")){b="***"
}if(i){i=this.sortObjects(i);if(!a){g=this.$input()[0];g.innerHTML=""}var e=this.get("emptyName");
if(e){if(h){e=e.loc()}if(a){c.push('<option value="***">%@</option>'.fmt(e));c.push('<option disabled="disabled"></option>')
}else{d=document.createElement("option");d.value="***";d.innerHTML=e;g.appendChild(d);
d=document.createElement("option");d.disabled="disabled";g.appendChild(d)}}i.forEach(function(m){if(m){var l=f?(m.get?m.get(f):m[f]):m.toString();
if(h){l=l.loc()}var n=(j)?(m.get?m.get(j):m[j]):m;if(n){n=(SC.guidFor(n))?SC.guidFor(n):n.toString()
}var k=(this.validateMenuItem&&this.validateMenuItem(n,l))?"":'disabled="disabled" ';
if(a){c.push('<option %@value="%@">%@</option>'.fmt(k,n,l))}else{d=document.createElement("option");
d.value=n;d.innerHTML=l;if(k.length>0){d.disable="disabled"}g.appendChild(d)}}else{if(a){c.push('<option disabled="disabled"></option>')
}else{d=document.createElement("option");d.disabled="disabled";g.appendChild(d)}}},this);
this.setFieldValue(b)}else{this.set("value",null)}}},displayProperties:["objects","nameKey","valueKey"],_objectsObserver:function(){this.set("cpDidChange",YES)
}.observes("objects"),_nameKeyObserver:function(){this.set("cpDidChange",YES)}.observes("nameKey"),_valueKeyObserver:function(){this.set("cpDidChange",YES)
}.observes("valueKey"),$input:function(){return this.$()},mouseDown:function(a){if(!this.get("isEnabled")){a.stop();
return YES}else{return arguments.callee.base.apply(this,arguments)}},getFieldValue:function(){var f=arguments.callee.base.apply(this,arguments);
var c=this.get("valueKey");var e=this.get("objects");var d;if(f=="***"){f=null}else{if(f&&e){var g=e.length;
d=null;while(!d&&(--g>=0)){var a=e[g];if(c){a=(a.get)?a.get(c):a[c]}var b=(a)?(SC.guidFor(a)?SC.guidFor(a):a.toString()):null;
if(f==b){d=a}}}}return(c||d)?d:f},setFieldValue:function(a){if(SC.none(a)){a=""}else{a=((a)?(SC.guidFor(a)?SC.guidFor(a):a.toString()):null)
}this.$input().val(a);return this},fieldDidFocus:function(){var a=this.get("isFocused");
if(!a){this.set("isFocused",true)}},fieldDidBlur:function(){var a=this.get("isFocused");
if(a){this.set("isFocused",false)}},_isFocusedObserver:function(){this.$().setClass("focus",this.get("isFocused"))
}.observes("isFocused"),didCreateLayer:function(){var a=this.$input();SC.Event.add(a,"blur",this,this.fieldDidBlur);
SC.Event.add(a,"focus",this,this.fieldDidFocus);return arguments.callee.base.apply(this,arguments)
},willDestroyLayer:function(){var a=this.$input();SC.Event.remove(a,"focus",this,this.fieldDidFocus);
SC.Event.remove(a,"blur",this,this.fieldDidBlur);return arguments.callee.base.apply(this,arguments)
}});SC.SliderView=SC.View.extend(SC.Control,{classNames:"sc-slider-view",handleSelector:"img.sc-handle",value:0.5,valueBindingDefault:SC.Binding.single().notEmpty(),minimum:0,minimumBindingDefault:SC.Binding.single().notEmpty(),contentMinimumKey:null,maximum:1,maximumBindingDefault:SC.Binding.single().notEmpty(),contentMaximumKey:null,step:0.1,displayProperties:"value minimum maximum".w(),render:function(d,g){arguments.callee.base.apply(this,arguments);
var c=this.get("minimum");var a=this.get("maximum");var f=this.get("value");f=Math.min(Math.max(f,c),a);
var e=this.get("step");if(!SC.none(e)&&e!==0){f=Math.round(f/e)*e}f=Math.floor((f-c)/(a-c)*100);
if(g){var b="/static/sproutcore/foundation/en/bcfdf5ca1125ccb312799938d8f09a81676f5db0/blank.gif";
d.push('<span class="sc-inner">');d.push('<span class="sc-leftcap"></span>');d.push('<span class="sc-rightcap"></span>');
d.push('<img src="',b,'" class="sc-handle" style="left: ',f,'%" />');d.push("</span>")
}else{this.$(this.get("handleSelector")).css("left",f+"%")}},_isMouseDown:NO,mouseDown:function(a){if(!this.get("isEnabled")){return YES
}this.set("isActive",YES);this._isMouseDown=YES;return this._triggerHandle(a)},mouseDragged:function(a){return this._isMouseDown?this._triggerHandle(a):YES
},mouseUp:function(a){if(this._isMouseDown){this.set("isActive",NO)}var b=this._isMouseDown?this._triggerHandle(a):YES;
this._isMouseDown=NO;return b},_triggerHandle:function(b){var g=this.convertFrameFromView({x:b.pageX}).x;
var e=this.get("frame").width;g=Math.max(Math.min(g,e-8),8)-8;e-=16;g=g/e;var d=this.get("minimum"),a=this.get("maximum");
var f=this.get("step"),c=this.get("value");g=d+((a-d)*g);if(f!==0){g=Math.round(g/f)*f
}if(Math.abs(c-g)>=0.01){this.set("value",g)}return YES},acceptsFirstResponder:function(){return this.get("isEnabled")
}.property("isEnabled"),willBecomeKeyResponderFrom:function(a){if(!this._isFocused){this._isFocused=YES;
this.becomeFirstResponder();if(this.get("isVisibleInWindow")){this.$()[0].focus()
}}},willLoseKeyResponderTo:function(a){if(this._isFocused){this._isFocused=NO}},keyDown:function(c){if(c.which===9){var b=c.shiftKey?this.get("previousValidKeyView"):this.get("nextValidKeyView");
b.becomeFirstResponder();return YES}if(c.which===37||c.which===38||c.which===39||c.which===40){var e=this.get("minimum"),a=this.get("maximum");
var f=this.get("step");var d=a-e,h=0,g;if(c.which===37||c.which===38){if(f===0){if(d<100){h=this.get("value")-1
}else{g=Math.abs(d/100);if(g<2){g=2}h=this.get("value")-Math.abs(d/100)}}else{h=this.get("value")-f
}}if(c.which===39||c.which===40){if(f===0){if(d<100){h=this.get("value")+2}else{g=Math.abs(d/100);
if(g<2){g=2}h=this.get("value")+g}}else{h=this.get("value")+f}}if(h>=e&&h<=a){this.set("value",h)
}}SC.RunLoop.begin().end();return YES},contentPropertyDidChange:function(c,a){var b=this.get("content");
this.beginPropertyChanges().updatePropertyFromContent("value",a,"contentValueKey",b).updatePropertyFromContent("minimum",a,"contentMinimumKey",b).updatePropertyFromContent("maximum",a,"contentMaximumKey",b).updatePropertyFromContent("isIndeterminate",a,"contentIsIndeterminateKey",b).endPropertyChanges()
}});sc_require("mixins/collection_group");sc_require("views/disclosure");SC.SourceListGroupView=SC.View.extend(SC.Control,SC.CollectionGroup,{classNames:["sc-source-list-group"],content:null,isGroupVisible:YES,hasGroupTitle:YES,groupTitleKey:null,groupVisibleKey:null,render:function(a,b){a.push('<a href="javascript:;" class="sc-source-list-label sc-disclosure-view sc-button-view button disclosure no-disclosure">');
a.push('<img src="%@" class="button" />'.fmt("/static/sproutcore/foundation/en/bcfdf5ca1125ccb312799938d8f09a81676f5db0/blank.gif"));
a.push('<span class="label"></span></a>')},createChildViews:function(){},contentPropertyDidChange:function(f,c){var e=this.get("content");
var h=this.outlet("labelView");if(e==null){h.setIfChanged("isVisible",NO);this.setIfChanged("hasGroupTitle",NO);
return}else{h.setIfChanged("isVisible",YES);this.setIfChanged("hasGroupTitle",YES)
}var b=this.getDelegateProperty("groupTitleKey",this.displayDelegate);if((c=="*")||(b&&(c==b))){var g=(e&&e.get&&b)?e.get(b):e;
if(g!=this._title){this._title=g;if(g){g=g.capitalize()}h.set("title",g)}}var d=this.getDelegateProperty("groupVisibleKey",this.displayDelegate);
if((c=="*")||(d&&(c==d))){if(d){h.removeClassName("no-disclosure");var a=(e&&e.get)?!!e.get(d):YES;
if(a!=this.get("isGroupVisible")){this.set("isGroupVisible",a);h.set("value",a)}}else{h.addClassName("no-disclosure")
}}},disclosureValueDidChange:function(c){if(c==this.get("isGroupVisible")){return
}var b=this.get("content");var a=this.getDelegateProperty("groupVisibleKey",this.displayDelegate);
if(b&&b.set&&a){b.set(a,c)}this.set("isGroupVisible",c);if(this.owner&&this.owner.updateChildren){this.owner.updateChildren(true)
}},labelView:SC.DisclosureView.extend({value:YES,_valueObserver:function(){if(this.owner){this.owner.disclosureValueDidChange(this.get("value"))
}}.observes("value")})});sc_require("views/list");sc_require("views/source_list_group");
SC.BENCHMARK_SOURCE_LIST_VIEW=YES;SC.SourceListView=SC.ListView.extend({classNames:["sc-source-list"],rowHeight:32,selectOnMouseDown:NO,actOnSelect:YES});
sc_require("views/split");SC.SplitDividerView=SC.View.extend({classNames:["sc-split-divider-view"],prepareContext:function(a,c){var b=this.get("splitView");
if(b){this.set("cursor",b.get("thumbViewCursor"))}return arguments.callee.base.apply(this,arguments)
},mouseDown:function(a){var b=this.get("splitView");return(b)?b.mouseDownInThumbView(a,this):arguments.callee.base.apply(this,arguments)
},doubleClick:function(a){console.log("doubleClick in split divider");var b=this.get("splitView");
return(b)?b.doubleClickInThumbView(a,this):arguments.callee.base.apply(this,arguments)
}});sc_require("views/split_divider");SC.RESIZE_BOTH="resize-both";SC.RESIZE_TOP_LEFT="resize-top-left";
SC.RESIZE_BOTTOM_RIGHT="resize-bottom-right";SC.SplitView=SC.View.extend({classNames:["sc-split-view"],childLayoutProperties:"layoutDirection dividerThickness autoresizeBehavior".w(),displayProperties:["layoutDirection"],delegate:null,layoutDirection:SC.LAYOUT_HORIZONTAL,canCollapseViews:YES,autoresizeBehavior:SC.RESIZE_BOTTOM_RIGHT,defaultThickness:0.5,isSplitView:YES,topLeftView:SC.View,dividerView:SC.SplitDividerView,bottomRightView:SC.View,topLeftThickness:function(){var a=this.get("topLeftView");
return a?this.thicknessForView(a):0}.property("topLeftView").cacheable(),bottomRightThickness:function(){var a=this.get("bottomRightView");
return a?this.thicknessForView(a):0}.property("bottomRightView").cacheable(),thumbViewCursor:null,canCollapseView:function(a){return this.invokeDelegateMethod(this.delegate,"splitViewCanCollapse",this,a)
},thicknessForView:function(a){var c=this.get("layoutDirection");var b=a.get("frame");
return(c===SC.LAYOUT_HORIZONTAL)?b.width:b.height},createChildViews:function(){var e=[];
var d=["topLeftView","dividerView","bottomRightView"];var c,b,a;for(b=0,a=d.length;
b<a;++b){if(c=this.get(d[b])){c=this[d[b]]=this.createChildView(c,{layoutView:this,rootElementPath:[b]});
e.push(c)}}this.set("childViews",e);return this},updateChildLayout:function(){var a=this.get("topLeftView");
var b=this.get("bottomRightView");var h=this.get("dividerView");var i=this.get("layoutDirection");
var d=this._desiredTopLeftThickness;var j=this.get("dividerThickness")||7;var g=(i==SC.LAYOUT_HORIZONTAL)?this.get("frame").width:this.get("frame").height;
var k=g-j-d;var c=this.get("autoresizeBehavior");var f;var e;e=a.get("isCollapsed")||NO;
a.setIfChanged("isVisible",!e);f=SC.clone(a.get("layout"));if(i==SC.LAYOUT_HORIZONTAL){f.top=0;
f.left=0;f.bottom=0;switch(c){case SC.RESIZE_BOTH:throw"SC.RESIZE_BOTH is currently unsupported.";
case SC.RESIZE_TOP_LEFT:f.right=k+j;delete f.width;break;case SC.RESIZE_BOTTOM_RIGHT:delete f.right;
delete f.height;f.width=d;break}}else{f.top=0;f.left=0;f.right=0;switch(c){case SC.RESIZE_BOTH:throw"SC.RESIZE_BOTH is currently unsupported.";
case SC.RESIZE_TOP_LEFT:f.bottom=k+j;delete f.height;break;case SC.RESIZE_BOTTOM_RIGHT:delete f.bottom;
delete f.width;f.height=d;break}}a.set("layout",f);if(h){f=SC.clone(h.get("layout"));
if(i==SC.LAYOUT_HORIZONTAL){f.width=j;delete f.height;f.top=0;f.bottom=0;switch(c){case SC.RESIZE_BOTH:throw"SC.RESIZE_BOTH is currently unsupported.";
case SC.RESIZE_TOP_LEFT:delete f.left;f.right=k;delete f.centerX;delete f.centerY;
break;case SC.RESIZE_BOTTOM_RIGHT:f.left=d;delete f.right;delete f.centerX;delete f.centerY;
break}}else{delete f.width;f.height=j;f.left=0;f.right=0;switch(c){case SC.RESIZE_BOTH:throw"SC.RESIZE_BOTH is currently unsupported.";
case SC.RESIZE_TOP_LEFT:delete f.top;f.bottom=k;delete f.centerX;delete f.centerY;
break;case SC.RESIZE_BOTTOM_RIGHT:f.top=d;delete f.bottom;delete f.centerX;delete f.centerY;
break}}h.set("layout",f)}e=b.get("isCollapsed")||NO;b.setIfChanged("isVisible",!e);
f=SC.clone(b.get("layout"));if(i==SC.LAYOUT_HORIZONTAL){f.top=0;f.bottom=0;f.right=0;
switch(c){case SC.RESIZE_BOTH:throw"SC.RESIZE_BOTH is currently unsupported.";case SC.RESIZE_BOTTOM_RIGHT:f.left=d+j;
delete f.width;break;case SC.RESIZE_TOP_LEFT:delete f.left;f.width=k;break}}else{f.left=0;
f.right=0;f.bottom=0;switch(c){case SC.RESIZE_BOTH:throw"SC.RESIZE_BOTH is currently unsupported.";
case SC.RESIZE_BOTTOM_RIGHT:f.top=d+j;delete f.height;break;case SC.RESIZE_TOP_LEFT:delete f.top;
f.height=k;break}}b.set("layout",f);this.notifyPropertyChange("topLeftThickness").notifyPropertyChange("bottomRightThickness")
},renderLayout:function(c,g){if(g){if(!this.get("thumbViewCursor")){this.set("thumbViewCursor",SC.Cursor.create())
}var f=this.get("layoutDirection");var b=(f==SC.LAYOUT_HORIZONTAL)?this.get("frame").width:this.get("frame").height;
var e=this.get("defaultThickness");var d=this.get("autoresizeBehavior");if(SC.none(e)||(e>0&&e<1)){e=Math.floor(b*(e||0.5))
}if(d===SC.RESIZE_BOTTOM_RIGHT){this._desiredTopLeftThickness=e}else{var a=this.get("dividerThickness")||7;
this._desiredTopLeftThickness=b-a-e}this._topLeftView=this.get("topLeftView");this._bottomRightView=this.get("bottomRightView");
this._topLeftViewThickness=this.thicknessForView(this.get("topLeftView"));this._bottomRightThickness=this.thicknessForView(this.get("bottomRightView"));
this._dividerThickness=this.get("dividerThickness");this._layoutDirection=this.get("layoutDirection");
this._updateTopLeftThickness(0);this._setCursorStyle();this.updateChildLayout()}arguments.callee.base.apply(this,arguments)
},render:function(b,c){arguments.callee.base.apply(this,arguments);if(this._inLiveResize){this._setCursorStyle()
}if(c){var a=this.get("layoutDirection");if(a===SC.LAYOUT_HORIZONTAL){b.addClass("sc-horizontal")
}else{b.addClass("sc-vertical")}}},mouseDownInThumbView:function(a,c){var b=this.getPath("pane.rootResponder");
if(!b){return NO}b.dragDidStart(this);this._mouseDownX=a.pageX;this._mouseDownY=a.pageY;
this._thumbView=c;this._topLeftView=this.get("topLeftView");this._bottomRightView=this.get("bottomRightView");
this._topLeftViewThickness=this.thicknessForView(this.get("topLeftView"));this._bottomRightThickness=this.thicknessForView(this.get("bottomRightView"));
this._dividerThickness=this.get("dividerThickness");this._layoutDirection=this.get("layoutDirection");
this.beginLiveResize();this._inLiveResize=YES;return YES},mouseDragged:function(a){var b=(this._layoutDirection==SC.LAYOUT_HORIZONTAL)?a.pageX-this._mouseDownX:a.pageY-this._mouseDownY;
this._updateTopLeftThickness(b);return YES},mouseUp:function(a){this._thumbView=null;
this._inLiveResize=NO;this.endLiveResize();return YES},doubleClickInThumbView:function(b,d){var a=this._topLeftView;
var c=a.get("isCollapsed")||NO;if(!c&&!this.canCollapseView(a)){a=this._bottomRightView;
c=a.get("isCollapsed")||NO;if(!c&&!this.canCollapseView(a)){return NO}}if(!c){this._uncollapsedThickness=this.getThicknessForView(a);
if(a===this._topLeftView){this._topLeftViewThickness=0}else{this._bottomRightViewThickness=0
}if(!a.get("isCollapsed")){this._uncollapsedThickness=null}}else{if(a===this._topLeftView){this._topLeftViewThickness=this._uncollapsedThickness
}else{this._bottomRightViewThickness=this._uncollapsedThickness}a._uncollapsedThickness=null
}this._setCursorStyle();return true},_updateTopLeftThickness:function(e){var a=this._topLeftView;
var c=this._bottomRightView;var f=this.thicknessForView(a);var g=this.thicknessForView(c);
var j=this._dividerThickness;var i=0;if(!a.get("isCollapsed")){i+=f}if(!c.get("isCollapsed")){i+=g
}var b=this._topLeftViewThickness+e;var m=this._layoutDirection;var o=this.canCollapseView(c);
var l=b;var k=this.get("topLeftMaxThickness");var d=this.get("topLeftMinThickness");
if(!SC.none(k)){l=Math.min(k,l)}if(!SC.none(d)){l=Math.max(d,l)}k=this.get("bottomRightMaxThickness");
d=this.get("bottomRightMinThickness");var n=i-l;if(!SC.none(k)){n=Math.min(k,n)}if(!SC.none(d)){n=Math.max(d,n)
}l=i-n;l=this.invokeDelegateMethod(this.delegate,"splitViewConstrainThickness",this,a,l);
l=Math.min(l,i);l=Math.max(0,l);var h=a.get("collapseAtThickness");if(!h){h=0}var p=c.get("collapseAtThickness");
p=SC.none(p)?i:(i-p);if((b<=h)&&this.canCollapseView(a)){k=c.get("maxThickness");
if(!k||(j+i)<=k){l=0}}else{if(b>=p&&this.canCollapseView(c)){k=a.get("maxThickness");
if(!k||(j+i)<=k){l=i}}}if(l!=this.thicknessForView(a)){this._desiredTopLeftThickness=l;
a.set("isCollapsed",l===0);c.set("isCollapsed",l>=i);this.updateChildLayout();this.displayDidChange()
}},_setCursorStyle:function(){var d=this._topLeftView;var e=this._bottomRightView;
var a=this.get("thumbViewCursor");this._layoutDirection=this.get("layoutDirection");
var b=this.thicknessForView(d);var c=this.thicknessForView(e);if(d.get("isCollapsed")||b==this.get("topLeftMinThickness")||c==this.get("bottomRightMaxThickness")){a.set("cursorStyle",this._layoutDirection==SC.LAYOUT_HORIZONTAL?"e-resize":"s-resize")
}else{if(e.get("isCollapsed")||b==this.get("topLeftMaxThickness")||c==this.get("bottomRightMinThickness")){a.set("cursorStyle",this._layoutDirection==SC.LAYOUT_HORIZONTAL?"w-resize":"n-resize")
}else{a.set("cursorStyle",this._layoutDirection==SC.LAYOUT_HORIZONTAL?"ew-resize":"ns-resize")
}}}.observes("layoutDirection"),splitViewCanCollapse:function(b,a){if(b.get("canCollapseViews")===NO){return NO
}if(a.get("canCollapse")===NO){return NO}return YES},splitViewConstrainThickness:function(c,a,b){return b
}});sc_require("views/collection");SC.StackedView=SC.CollectionView.extend({classNames:["sc-stacked-view"],layout:{top:0,left:0,right:0,height:1},updateHeight:function(a){if(a){this._updateHeight()
}else{this.invokeLast(this._updateHeight)}return this},_updateHeight:function(){var e=this.get("childViews"),b=e.get("length"),c,d,a;
if(b===0){a=1}else{c=e.objectAt(b-1);d=c?c.get("layer"):null;a=d?(d.offsetTop+d.offsetHeight):1;
d=null}this.adjust("height",a)},didReload:function(a){return this.updateHeight()},didCreateLayer:function(){return this.updateHeight()
}});sc_require("views/segmented");SC.TOP_LOCATION="top";SC.BOTTOM_LOCATION="bottom";
SC.TabView=SC.View.extend({classNames:["sc-tab-view"],displayProperties:["nowShowing"],nowShowing:null,items:[],isEnabled:YES,itemTitleKey:null,itemValueKey:null,itemIsEnabledKey:null,itemIconKey:null,itemWidthKey:null,tabLocation:SC.TOP_LOCATION,userDefaultKey:null,_tab_nowShowingDidChange:function(){var a=this.get("nowShowing");
this.get("containerView").set("nowShowing",a);this.get("segmentedView").set("value",a);
return this}.observes("nowShowing"),_tab_saveUserDefault:function(){var a=this.get("nowShowing");
var b=this.get("userDefaultKey");if(b){SC.userDefaults.set([b,"nowShowing"].join(":"),a)
}}.observes("nowShowing"),_tab_itemsDidChange:function(){this.get("segmentedView").set("items",this.get("items"));
return this}.observes("items"),init:function(){arguments.callee.base.apply(this,arguments);
this._tab_nowShowingDidChange()._tab_itemsDidChange()},awake:function(){arguments.callee.base.apply(this,arguments);
var a=this.get("userDefaultKey");if(a){a=[a,"nowShowing"].join(":");var b=SC.userDefaults.get(a);
if(!SC.none(b)){this.set("nowShowing",b)}}},createChildViews:function(){var c=[],b,a;
if(this.get("tabLocation")===SC.TOP_LOCATION){a=this.containerView.extend({layout:{top:12,left:0,right:0,bottom:0}})
}else{a=this.containerView.extend({layout:{top:0,left:0,right:0,bottom:12}})}b=this.containerView=this.createChildView(a);
c.push(b);b=this.segmentedView=this.createChildView(this.segmentedView);c.push(b);
this.set("childViews",c);return this},containerView:SC.ContainerView,segmentedView:SC.SegmentedView.extend({layout:{left:0,right:0,height:24},_sc_tab_segmented_valueDidChange:function(){var a=this.get("parentView");
if(a){a.set("nowShowing",this.get("value"))}this.set("layerNeedsUpdate",YES);this.invokeOnce(this.updateLayerIfNeeded)
}.observes("value"),render:function(b,d){arguments.callee.base.apply(this,arguments);
var c=this.get("parentView");var a=(c)?c.get("tabLocation"):SC.TOP_LOCATION;if(a===SC.TOP_LOCATION){b.addStyle("top","0px")
}else{b.addStyle("bottom","0px")}},init:function(){var a=this.get("parentView");if(a){SC._TAB_ITEM_KEYS.forEach(function(b){this[b]=a.get(b)
},this)}return arguments.callee.base.apply(this,arguments)}})});SC._TAB_ITEM_KEYS="itemTitleKey itemValueKey itemIsEnabledKey itemIconKey itemWidthKey".w();
SC.ThumbView=SC.View.extend({classNames:["sc-thumb-view"],isEnabled:YES,isEnabledBindingDefault:SC.Binding.bool(),prepareContext:function(a,c){var b=this.get("splitView");
if(b){this.set("cursor",b.get("thumbViewCursor"))}return arguments.callee.base.apply(this,arguments)
},mouseDown:function(a){if(!this.get("isEnabled")){return NO}var b=this.get("splitView");
return(b)?b.mouseDownInThumbView(a,this):arguments.callee.base.apply(this,arguments)
}});SC.ToolbarView=SC.View.extend({classNames:["sc-toolbar-view"],anchorLocation:null,layout:{left:0,height:32,right:0},init:function(){if(this.anchorLocation){this.layout=SC.merge(this.layout,this.anchorLocation)
}arguments.callee.base.apply(this,arguments)}});SC.WebView=SC.View.extend(SC.Control,{classNames:"sc-web-view",displayProperties:["value","shouldAutoResize"],shouldAutoResize:NO,render:function(a,d){var c=this.get("value");
if(d){a.push('<iframe src="'+c+'" style="position: absolute; width: 100%; height: 100%; border: 0px; margin: 0px; padding: 0p;"></iframe>')
}else{var b=this.$("iframe");b.attr("src","javascript:;");b.attr("src",c)}},didCreateLayer:function(){var a=this.$("iframe");
SC.Event.add(a,"load",this,this.iframeDidLoad)},iframeDidLoad:function(){if(this.get("shouldAutoResize")===YES){var a;
var c=this.$("iframe")[0];if(c&&c.contentWindow){a=c.contentWindow;if(a&&a.document&&a.document.documentElement){var b=a.document.documentElement;
if(!SC.browser.isIE){this.$().width(b.scrollWidth);this.$().height(b.scrollHeight)
}else{this.$().width(b.scrollWidth+12);this.$().height(b.scrollHeight+5)}}}}}});if((typeof SC!=="undefined")&&SC&&SC.bundleDidLoad){SC.bundleDidLoad("sproutcore/desktop")
};