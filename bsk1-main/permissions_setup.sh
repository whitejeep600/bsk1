setfacl -m g:clients:r deposits
setfacl -m g:clients:r credits
setfacl -m g:officers:rwx deposits
setfacl -m g:officers:rwx credits
setfacl -d -m g:officers:rw deposits
setfacl -d -m g:officers:rw credits
setfacl -d -m g:clients:--- deposits
setfacl -d -m g:clients:--- credits

