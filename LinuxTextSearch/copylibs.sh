


([ -f ./TextSearch/dist/Debug/GNU-Linux/libTextSearch.so ] && echo "\nFound libTextSearch.so .. \nMoving libTextSearch.so to /usr/lib/" &&
sudo cp ./TextSearch/dist/Debug/GNU-Linux/libTextSearch.so /usr/lib) || (echo "\nCould not find libTextSearch.so..\nBuilding TextSearch.." && cd ./TextSearch && make && cd .. && echo "\nMoving libTextSearch.so to /usr/lib/" && sudo cp ./TextSearch/dist/Debug/GNU-Linux/libTextSearch.so /usr/lib)

([ -f ./FileManager/dist/Debug/GNU-Linux/libFileManager.so ] && echo "\nFound libFileManager.so .. \nMoving libFileManager.so to /usr/lib/" &&
sudo cp ./FileManager/dist/Debug/GNU-Linux/libFileManager.so /usr/lib) || (echo "\nCould not find libFileManager.so..\nBuilding FileManager.." && cd ./FileManager && make && cd .. && echo "Moving libFileManager.so to /usr/lib/" && sudo cp ./FileManager/dist/Debug/GNU-Linux/libFileManager.so /usr/lib)
