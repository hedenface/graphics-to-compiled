# graphics to compiled

> Take bitmap images and convert them to data arrays to be compiled into your C/C++ project

It also can make specific colors transparent during the conversion (e.g.: BMP -> OpenGL RGBA)

## Compile

```
gcc -g g2c.c -o g2c
```

## Execute

```
./g2c squares
```

**Note:** specifying `squares` as first argument requires that there is a file `squares.bmp` and `squares.info`. See included example files.
