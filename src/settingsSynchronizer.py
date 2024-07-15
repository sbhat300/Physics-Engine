code = open('src/collisions.cpp', 'r+')
if code:
    print('Code file opened')
data = open('src/collisionsObjectData.txt', 'r')
if data:
    print('Data file opened')
code_inp_raw = code.read()
grid_name = code_inp_raw.split('spatialHashGrid')[2].split('(')[0].strip()
code_inp = code_inp_raw.split('/*-----ENTITY INITIALIZATION-----*/')[1].split('/*-----END-----*/', 1)[0].strip()
objects = {}
lines = code_inp.split('entity ')[1:]
for i, line in enumerate(lines):
    objects[str(i)] = line.split('(')[0]
entity_outputs = []
polygon_outputs = []
polygon_collider_outputs = []
for line in data:
    if line == '':
        break
    if line == '\n':
        continue
    raw = line.strip().split(' ')
    posSplit = raw[1][1:].split(',')
    scaleSplit = raw[2][1:].split(',')
    entity_outputs.insert(int(raw[0]), ' glm::vec2(' + posSplit[0] + ", " + posSplit[1] + '), glm::vec2(' + scaleSplit[0] + ', ' + scaleSplit[1] + "), " + raw[3][1:])
    if raw[4][0] != 'x':
        posSplit = raw[4][2:].split(',')
        scaleSplit = raw[5][2:].split(',')
        colSplit = raw[7][3:].split(',')
        polygon_outputs.append(objects[raw[0]] + '.addPolygon(glm::vec2(' + posSplit[0] + ", " + posSplit[1] + '), glm::vec2(' + scaleSplit[0] + ', ' + scaleSplit[1] + "), " + raw[6][2:] + ', glm::vec3(' + colSplit[0] + ', ' + colSplit[1] + ', ' + colSplit[2] + '), ' + raw[8][1:] + ');')
    if raw[9][0] != 'x':
        posSplit = raw[9][3:].split(',')
        scaleSplit = raw[10][3:].split(',')
        polygon_collider_outputs.append(objects[raw[0]] + '.addPolygonCollider(&' + grid_name + ', glm::vec2(' + posSplit[0] + ', ' + posSplit[1] + '), glm::vec2(' + scaleSplit[0] + ', ' + scaleSplit[1] + ')' + ', ' + raw[11][3:] + ');')
code.seek(0)
code_data = code.readlines()
start_entity = False
start_poly = False
start_collider = False
started_at = 0
counter = 0
for i, line in enumerate(code_data):
    inp = line.strip()
    if inp == '/*-----END-----*/':
        counter += 1
        start_entity = False
        start_poly = False
        start_collider = False
        if counter == 3:
            break
    if start_entity:
        splitted = inp.split(',')
        splitted = splitted[0:1] + [entity_outputs[i - started_at - 1]] + splitted[6:]
        code_data[i] = ",".join(splitted) + '\n'
    if start_poly:
        code_data[i] = '\t' + polygon_outputs[i - started_at - 1] + '\n'
    if start_collider:
        code_data[i] = '\t' + polygon_collider_outputs[i - started_at - 1] + '\n'
    if inp == '/*-----ENTITY INITIALIZATION-----*/':
        start_entity = True
        started_at = i
    if inp == '/*-----POLYGON INITIALIZATION-----*/':
        start_poly = True
        started_at = i
    if inp == '/*-----COLLIDER INITIALIZATION-----*/':
        start_collider = True
        started_at = i
code.seek(0)
code.writelines(code_data)
code.close()
data.close()
print("done")