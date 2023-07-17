add_definitions(-DVIDEOCALL_SCENE)

include_directories(
  ${PORJECT_ROOT_PATH}/videocall
)

file (GLOB_RECURSE VIDEOCALL_SOURCE  
	${PORJECT_ROOT_PATH}/videocall/*.cc 
	${PORJECT_ROOT_PATH}/videocall/*.h 
	${PORJECT_ROOT_PATH}/videocall/*.ui
)

set(PROJECT_SRC 
	${PROJECT_SRC}
	${VIDEOCALL_SOURCE}
)

set(PROJECT_QRC 
	${PROJECT_QRC}
    ${PORJECT_ROOT_PATH}/videocall/resource/video_call_resource.qrc
)

file(GLOB VIDEOCALL_TS_FILES ${PORJECT_ROOT_PATH}/videocall/resource/translations/*.ts)
# 遍历所有 .ts 文件，为每个文件生成一个 .qm 文件
foreach(_ts_file ${VIDEOCALL_TS_FILES})
	get_filename_component(_ts_file_name ${_ts_file} NAME_WE)
	add_custom_command(
		OUTPUT "${PORJECT_ROOT_PATH}/videocall/resource/translations/${_ts_file_name}.qm"
		COMMAND ${QT_LRELEASE_EXECUTABLE} ${_ts_file} -qm "${PORJECT_ROOT_PATH}/videocall/resource/translations/${_ts_file_name}.qm"
		DEPENDS ${_ts_file}
		COMMENT "Generating ${_ts_file_name}.qm"
	)
endforeach()