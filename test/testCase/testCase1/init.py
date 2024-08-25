csvContent = "ID,Username,Email\n"
for i in range(1, 251):
    csvContent += f"{i},{i},{i}\n"

with open("/home/marco/0_codeRepo/2_project/3_cppProject/4_tinytinyDB/demo/demo_0/test/bin/standard_export.csv", "w+") as file:
    file.write(csvContent)
    
###########################################

insertCmd = ""
for i in range(1, 251):
    insertCmd += f"insert {i} {i} {i}\n"
insertCmd += ".exit\n"

with open("/home/marco/0_codeRepo/2_project/3_cppProject/4_tinytinyDB/demo/demo_0/test/cmd/insertCmd", "w+") as file:
    file.write(insertCmd)