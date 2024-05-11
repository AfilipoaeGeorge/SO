 #!/bin/bash

# Argumentul scriptului este calea către fișierul de analizat
file_path="$1"

# Verificăm dacă fișierul există
if [ ! -f "$file_path" ]; then
    echo "Error: file not found"
    exit -1
fi

chmod 700 "$file_path"
num_chars=$(wc -m < "$file_path")
num_lines=$(wc -l < "$file_path")
num_words=$(wc -w < "$file_path")

#Conditii linii,cuvinte,caractere(nr)
if [ "$num_lines" -lt 3 ] && [ "$num_words" -gt 1000 ] && [ "$num_chars" -gt 2000 ]; then
    echo "$file_path"
    chmod 000 "$file_path"
    exit 1
fi

# Verificăm dacă fișierul conține caractere non-ASCII
if grep -qP '[^\x00-\x7F]' "$file_path"; then
    echo "$file_path"
    chmod 000 "$file_path"
    exit 1
fi

# Verificăm dacă fișierul există si daca el contine cuvinte specifice
if [ -f "$file_path" ]; then
   while IFS= read -r linieFisier || [ -n "$linieFisier" ]; do
    if echo "$linieFisier" | grep -qE 'corrupted|dangerous|risk|attack|malware|malicious'; then
        chmod 000 "$file_path"
        echo "$file_path"
        exit 1
        break
    fi
done < "$file_path"
fi

echo "SAFE"
chmod 000 "$1"
exit 2