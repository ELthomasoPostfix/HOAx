OUT_DIR=output
OUT_EXE_NAME=main.out
OUT_EXE=$OUT_DIR/$OUT_EXE_NAME

if [ -d $OUT_DIR ];
then
  echo "compiling ..."
  g++ -std=c++17 src/main.cpp -lspot -o $OUT_EXE
  echo "generated executable at $OUT_EXE"
  echo "exiting ..."
else
  echo "Output directory \"$OUT_DIR\" does not exist. Failed to compile. "\
       "Did you run from the project root?"
fi
