from itertools import permutations

def generate_partitions(elements):
    """Generate all unordered partitions of a set of elements."""
    if not elements:
        yield []
        return
    
    first = elements[0]
    rest = elements[1:]
    
    for partition in generate_partitions(rest):
        yield [[first]] + partition
        
        for i in range(len(partition)):
            new_partition = [block[:] for block in partition]
            new_partition[i] = [first] + new_partition[i]
            yield new_partition

def generate_ordered_partitions(elements):
    """Generate all ordered partitions (all permutations of all partitions)."""
    for partition in generate_partitions(elements):
        for perm in permutations(partition):
            yield list(perm)

def generate_cpp_code(start_idx, end_idx, vec_prefix="boundedVec", bitset_prefix="bounded"):
    """Generate C++ code for all ordered partitions of elements in [start_idx, end_idx]."""
    elements = list(range(start_idx, end_idx + 1))
    bitset_size = end_idx + 1
    
    ordered_partitions = list(generate_ordered_partitions(elements))
    
    cpp_code = []
    cpp_code.append(f"// Total ordered partitions: {len(ordered_partitions)}")
    cpp_code.append(f"// Elements: {elements}")
    cpp_code.append("")
    
    global_bitset_counter = 0
    
    for partition_idx, partition in enumerate(ordered_partitions):
        cpp_code.append(f"// Ordered partition {partition_idx}: {partition}")
        cpp_code.append("{")
        
        bitset_names = []
        for block in partition:
            bitset_name = f"{bitset_prefix}{global_bitset_counter}"
            bitset_names.append(bitset_name)
            cpp_code.append(f"    boost::dynamic_bitset<> {bitset_name}({bitset_size});")
            for elem in block:
                cpp_code.append(f"    {bitset_name}[{elem}] = true;")
            global_bitset_counter += 1
        
        cpp_code.append("")
        vec_name = f"{vec_prefix}{partition_idx}"
        cpp_code.append(f"    std::deque<boost::dynamic_bitset<>> {vec_name};")
        
        for bitset_name in reversed(bitset_names):
            cpp_code.append(f"    {vec_name}.push_front({bitset_name});")
        
        cpp_code.append("}")
        cpp_code.append("")
    
    return "\n".join(cpp_code)

if __name__ == "__main__":
    import sys
    
    if len(sys.argv) == 3:
        start_idx = int(sys.argv[1])
        end_idx = int(sys.argv[2])
    elif len(sys.argv) == 4:
        start_idx = int(sys.argv[1])
        end_idx = int(sys.argv[2])
        output_file = sys.argv[3]
    else:
        start_idx = 2
        end_idx = 5
    
    if len(sys.argv) != 4:
        output_file = f"ordered_partitions_{start_idx}_{end_idx}.txt"
    
    cpp_code = generate_cpp_code(start_idx, end_idx)
    
    with open(output_file, 'w') as f:
        f.write(cpp_code)
    
    print(f"Generated C++ code written to: {output_file}")