echo ">>> Building assembler and reference simulator .."
cd architecture-tools/
make
cd ..
echo ">>> Building simulator .."
cd src/
make
cd ..
cd tests/
echo ""

#echo ">>> Testing hlt.."
#../architecture-tools/asm ./hlt.x64 ./hlt.out
#../architecture-tools/sim ./hlt.out ./hlt.trace
#../src/sim ./hlt.out ./hlt.trace
#echo ""

# echo ">>> Testing nop.."
# ../architecture-tools/asm ./nop.x64 ./nop.out
# ../architecture-tools/sim ./nop.out nop.trace
# ../src/sim ./nop.out ./nop.trace
# echo ""

# echo ">>> Testing movq_RtoR.."
# ../architecture-tools/asm ./movq_RtoR.x64 ./movq_RtoR.out
# ../architecture-tools/sim ./movq_RtoR.out movq_RtoR.trace
# ../src/sim ./movq_RtoR.out ./movq_RtoR.trace
# echo ""

# echo ">>> Testing movq_ItoR.."
# ../architecture-tools/asm ./movq_ItoR.x64 ./movq_ItoR.out
# ../architecture-tools/sim ./movq_ItoR.out movq_ItoR.trace
# ../src/sim ./movq_ItoR.out ./movq_ItoR.trace
# echo ""

# echo ">>> Testing movq_RtoM.."
# ../architecture-tools/asm ./movq_RtoM.x64 ./movq_RtoM.out
# ../architecture-tools/sim ./movq_RtoM.out movq_RtoM.trace
# ../src/sim ./movq_RtoM.out ./movq_RtoM.trace
# echo ""

# echo ">>> Testing movq_MtoR.."
# ../architecture-tools/asm ./movq_MtoR.x64 ./movq_MtoR.out
# ../architecture-tools/sim ./movq_MtoR.out movq_MtoR.trace
# ../src/sim ./movq_MtoR.out ./movq_MtoR.trace
# echo ""

#  echo ">>> Testing compq.."
#  ../architecture-tools/asm ./compq.x64
#  ../architecture-tools/asm ./compq.x64 ./compq.out
#  ../architecture-tools/sim ./compq.out ./compq.trace
#  ../src/sim ./compq.out ./compq.trace
#  echo ""

# echo ">>> Testing call.."
# ../architecture-tools/asm ./call_ret.x64
# ../architecture-tools/asm ./call_ret.x64 ./call_ret.out
# ../architecture-tools/sim ./call_ret.out ./call_ret.trace
# ../src/sim ./call_ret.out ./call_ret.trace
# echo ""

# echo ">>> Testing push_pop.."
# ../architecture-tools/asm ./push_pop.x64 ./push_pop.out
# ../architecture-tools/sim ./push_pop.out ./push_pop.trace
# ../src/sim ./push_pop.out ./push_pop.trace
# echo ""

# echo ">>> Testing movq_bug.."
# ../architecture-tools/asm ./movq_bug.x64 ./movq_bug.out
# ../architecture-tools/sim ./movq_bug.out movq_bug.trace
# ../src/sim ./movq_bug.out ./movq_bug.trace
# echo ""

# echo ">>> Testing arithmetic_test.."
# ../architecture-tools/asm ./arithmetic_test.x64
# ../architecture-tools/asm ./arithmetic_test.x64 ./arithmetic_test.out
# ../architecture-tools/sim ./arithmetic_test.out arithmetic_test.trace
# ../src/sim ./arithmetic_test.out ./arithmetic_test.trace
# echo ""
