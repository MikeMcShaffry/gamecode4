
local classes = ...
local ret1 = {}

-- don't bind this Qt internals/unsupported classes
-- if there are linker errors, or errors when laoding the .so 
-- add the class here 

for c in pairs(classes) do
	local n = c.xarg.name
	if n~=string.lower(n) and not (string.match(n, '_')
			-- these are useless to bind, but compile
			or c.xarg.fullname=='QVariant::Private' -- well, it IS public
			or c.xarg.fullname=='QVariant::Private::Data' -- well, it IS public
			or c.xarg.fullname=='QVariant::PrivateShared' -- well, it IS public
			or c.xarg.fullname=='QObjectData'-- compiles
			or c.xarg.fullname=='QtConcurrent::internal::ExceptionStore' -- it compiles
			or c.xarg.fullname=='QtConcurrent::internal::ExceptionHolder' -- it compiles
			or c.xarg.fullname=='QtConcurrent::ResultIteratorBase' -- it compiles
			or c.xarg.fullname=='QtSharedPointer' -- compiles
			or c.xarg.fullname=='QtSharedPointer::InternalRefCountData' -- compiles
			or c.xarg.fullname=='QtSharedPointer::ExternalRefCountData' -- compiles
			or c.xarg.fullname=='QUpdateLaterEvent' -- compiles
			or c.xarg.fullname=='QTextStreamManipulator' -- compiles
			or c.xarg.fullname=='QtConcurrent::ThreadEngineSemaphore' -- compiles
			or c.xarg.fullname=='QtConcurrent::ThreadEngineBarrier' -- linker error
			
			-- platform specific, TODO
			or c.xarg.fullname=='QWindowsCEStyle'
			or c.xarg.fullname=='QWindowsMobileStyle'
			or c.xarg.fullname=='QWindowsXPStyle'
			or c.xarg.fullname=='QWindowsVistaStyle'
			or c.xarg.fullname=='QMacStyle'
			or c.xarg.fullname=='QS60Style'
			or c.xarg.fullname=='QS60MainApplication'
			or c.xarg.fullname=='QS60MainAppUI'
			or c.xarg.fullname=='QS60MainDocument'
			or c.xarg.fullname=='QWSCalibratedMouseHandler'
			or c.xarg.fullname=='QWSClient'
			or c.xarg.fullname=='QWSEmbedWidget'
			or c.xarg.fullname=='QWSEvent'
			or c.xarg.fullname=='QWSGLWindowSurface'
			or c.xarg.fullname=='QWSInputMethod'
			or c.xarg.fullname=='QWSKeyboardHandler'
			or c.xarg.fullname=='QWSMouseHandler'
			or c.xarg.fullname=='QWSPointerCalibrationData'
			or c.xarg.fullname=='QWSScreenSaver'
			or c.xarg.fullname=='QWSServer'
			or c.xarg.fullname=='QWSWindow'
			or c.xarg.fullname=='QXmlNodeModelIndex' -- a method "name" is public but is not part of the documented API
			or c.xarg.fullname=='QXmlName' -- a method "localName" is public but is not part of the documented API

			-- binding bugs
			or c.xarg.fullname=='QThreadStorageData' -- binding error (function pointer)
			or c.xarg.fullname=='QForeachContainerBase' -- "was not declared in this scope"
			or c.xarg.fullname=='QFutureWatcherBase' -- const virtual method causes it to be abstract
			or c.xarg.fullname=='QEasingCurve'        -- wrapper for function: function pointer parsing problem
			or c.xarg.fullname=='QHashData'        -- not in the docs at all. free_helper is not present during compilation
			or string.match(c.xarg.fullname, '^QtConcurrent') -- does not make sense anyway, because we should duplicate the lua_State
			or string.match(c.xarg.fullname, '^QAccessible') -- causes a lot of headaches, and not necessarry anyway (yet)
			or string.match(c.xarg.fullname, 'Private$') -- should not bind these
			) then
		ret1[c] = true
	else
		ignore(c.xarg.fullname, "blacklisted", "filter")
	end
end

return ret1

