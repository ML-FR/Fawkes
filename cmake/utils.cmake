function(check_deps_extra_libs libs)
	set(EXTRA_LIBS_WARNING)
  foreach(lib ${libs})
    if(NOT ${lib}_FOUND)
      pkg_check_modules(${lib} ${lib})
      if(NOT ${lib}_FOUND)
				list(APPEND EXTRA_LIBS_WARNING "${lib} dependency missing")
      endif()
    endif()
  endforeach()
	set(EXTRA_LIBS_WARNING
		${EXTRA_LIBS_WARNING}
      PARENT_SCOPE)
endfunction()

function(depend_on_extra_libs target libs)
  foreach(lib ${libs})
    if(${lib}_FOUND)
      target_link_libraries(${target} ${${lib}_LDFLAGS})
      target_compile_options(${target} PUBLIC ${${lib}_CFLAGS})
    endif()
  endforeach()
endfunction()
