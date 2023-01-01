# b generatorC.cpp:232
# r
# display max_x

b generatorC.cpp:344
condition 1 closest_voro_id == 177
b generatorC.cpp:369
condition 2 closest_voro_id == 177 && closest_neighbor_id != 105

disable 1
r
display {closest_voro_id, closest_neighbor_id}
display
