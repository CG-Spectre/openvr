from stl import mesh

# Load the STL file
your_mesh = mesh.Mesh.from_file('monkey.stl')
outputFaces = ""
outputVertices = ""
count = 0
print("Converting...")
# Each triangle (face) in an STL has 3 vertices
for i, triangle in enumerate(your_mesh.vectors):
    entryStr = ""
    for j, vertex in enumerate(triangle):
        outputVertices += f"{vertex[0]} {vertex[1]} {vertex[2]}\n"
        entryStr += f"{count} "
        count += 1

    outputFaces += entryStr + "\n"

f = open("test.vrobj", "w")
f.write(f"{outputVertices}\n{outputFaces}")
f.close()
print("Done.")