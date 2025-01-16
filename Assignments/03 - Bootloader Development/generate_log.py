import sys


def main():
    if len(sys.argv) < 2:
        print("Usage: %s <in_file> [out_file]\n" % sys.argv[0])
        return

    filename_in = sys.argv[1]

    if len(sys.argv) > 2:
        filename_out = sys.argv[2]
    else:
        filename_out = filename_in + ".bin"

    try:
        with open(filename_in, "rb") as file_in:
            file_bytes = file_in.read()
    except:
        print("Error: could not read from %s.\n" % filename_in)
        return

    try:
        with open(filename_out, "wb") as file_out:
            file_out.write(file_bytes)
            file_out.write(bytearray([0, 0]))

    except:
        print("Error: could not write to %s.\n" % filename_out)
        return

    print("Succesfully wrote %s from %s.\n" % (filename_out, filename_in))


if __name__ == "__main__":
    main()
