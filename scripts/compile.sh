OUT_DIR=$1
OUT_EXE_NAME=$2
OUT_EXE=$OUT_DIR/$OUT_EXE_NAME

if [ ! $# -eq 2 ]; then
    echo "Provided $# arguments. Expected 2 arguments."
    exit 1
fi

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
