# - Vivante headers and libraries
#set (Vivante_INC_SEARCH_PATH "usr/include")
find_path (Vivante_G2D_INCLUDE_DIR g2d.h
	PATHS ${Vivante_INC_SEARCH_PATH}
	DOC "The directory where gd2.h resides"
	)

find_library (Vivante_G2D_LIBRARY libg2d.so
	PATHS ${Vivante_LIB_SEARCH_PATH}
	DOC "The directory where libg2d resides"
	)
find_library (Vivante_VDK_LIBRARY libVDK.so
	PATHS ${Vivante_LIB_SEARCH_PATH}
	DOC "The directory where libVDK resides"
	)

if (Vivante_G2D_INCLUDE_DIR AND Vivante_G2D_LIBRARY AND Vivante_VDK_LIBRARY)
	set (Vivante_FOUND 1)
endif ()

mark_as_advanced (
	Vivante_G2D_INCLUDE_DIR
	Vivante_G2D_LIBRARY
	Vivante_VDK_LIBRARY
)

mark_as_advanced (
	Vivante_FOUND
)
