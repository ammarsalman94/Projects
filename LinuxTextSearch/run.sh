sudo cp ./FileManager/dist/Debug/GNU-Linux/libFileManager.so /usr/lib
sudo cp ./TextSearch/dist/Debug/GNU-Linux/libTextSearch.so /usr/lib

echo "\n\n Running ./textsearchclient "
./textsearchclient 

echo "\n\n Running ./textsearchclient --path ./test --search ammar --patterns \"*.h *.txt\""
./textsearchclient --path ./test --search ammar --patterns "*.h *.txt" 

echo "\n\n Running ./textsearchclient --path ./test --search ammar --patterns \"*.txt\" -subs "
./textsearchclient --path ./test --search ammar --patterns "*.txt" -subs

echo "\n\n Running ./textsearchclient --path ./test --search nothing --patterns \"*.txt\" -subs "
./textsearchclient --path ./test --search nothing --patterns "*.txt" -subs
