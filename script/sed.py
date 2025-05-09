import sys, os

def sed_file(flie_name, m):
    lines = []
    with open(flie_name, "r") as file:
        lines = file.readlines()

    if not m(lines):
        return

    with open(flie_name, "w") as file:
        for line in lines:
            file.write(line)
  

if __name__ == "__main__":
    if len(sys.argv) < 4:
        print("Usage: python sed.py <filename>")
        sys.exit(1)
    num_line = int(sys.argv[1])
    new_content = sys.argv[2]
    filename = sys.argv[3]
    def modify_line(lines):
        modified = False
        if len(lines) < num_line:
            for i in range(num_line - len(lines)):
                lines.append("\n")
                modified = True
        for i in range(len(lines)):
            if i + 1 == num_line:
                if lines[i] == new_content + "\n":
                    continue
                lines[i] = new_content + "\n"
                modified = True
        return modified

    sed_file(filename, modify_line)
