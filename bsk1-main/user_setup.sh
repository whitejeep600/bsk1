while read -a line; do
useradd -m ${line[0]} -c "${line[2]} ${line[3]}"
usermod -a -G "${line[1]}s" ${line[0]}
echo "${line[0]}:${line[0]}" | chpasswd # setting usernames as default passwords
done < uzytkownicy.txt
