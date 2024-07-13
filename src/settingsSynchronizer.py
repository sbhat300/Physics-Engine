code = open('src/collisions.cpp', 'r+')
if code:
    print('Code file opened')
data = open('src/collisionsObjectData.txt', 'r')
if data:
    print('Data file opened')
code_inp = code.read().split('/*-----ENTITY INITIALIZATION-----*/')[1].split('/*-----END-----*/')[0].strip()
objects = {}
lines = code_inp.split('entity ')[1:]
for i, line in enumerate(lines):
    objects[str(i)] = line.split('(')[0]  
outputs = []
for line in data:
    if line == '':
        break
    if line == '\n':
        continue
    raw = line.strip().split(' ')
    posSplit = raw[1][1:].split(',')
    scaleSplit = raw[2][1:].split(',')
    outputs.insert(int(raw[0]), ' glm::vec2(' + posSplit[0] + ", " + posSplit[1] + '), glm::vec2(' + scaleSplit[0] + ', ' + scaleSplit[1] + "), " + raw[3][1:])
code.seek(0)
code_data = code.readlines()
start = False
startedAt = 0
for i, line in enumerate(code_data):
    inp = line.strip()
    if inp == '/*-----END-----*/':
        break
    if start:
        splitted = inp.split(',')
        splitted = splitted[0:1] + [outputs[i - startedAt - 1]] + splitted[6:]
        code_data[i] = ",".join(splitted) + '\n'
    if inp == '/*-----ENTITY INITIALIZATION-----*/':
        start = True
        startedAt = i
code.seek(0)
code.writelines(code_data)
print("done")