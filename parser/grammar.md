config file:

server : listen[optional] {...}, server_name[optional] {...}, index[optional], location {...}, root[optional], key_words[allow_methods, body_limit]; autoindex[optional]

listen : port | host, host:port;

server_name : string {...};

index : document_name;

location : path_name + { root[optional], index[optional], autoindex, key_words {...}, }

autoindex : on | off



