waf_path="../waf"

nMpduList=("1" "2" "4" "8" "16" "32")

# export "NS_LOG=Exercise6=level_info"
for nMpdu in ${nMpduList[@]}; do
    ${waf_path} --run "Exercise6 --nMpdu=${nMpdu}"
done