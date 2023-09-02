set(test_folder "Test")

add_subdirectory(test)
set_target_properties(test PROPERTIES FOLDER ${test_folder}/test)


add_subdirectory(sdf)
set_target_properties(sdf PROPERTIES FOLDER ${test_folder}/sdf)
