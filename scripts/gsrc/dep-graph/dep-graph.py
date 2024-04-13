# goalc can generate a project wide symbol definition map
# - global vars
# - constants
# - functions
    # first arg in list form
# - types
# - macros
    # first arg in list form
# - method
    # first arg in list form
    # can't reliable do this one, need to bake something into goalc
    # same for states (virtual ones atleast)
        # TODO - add states to the symbol_map!

# NOTE, caveats:
# - macros, ideally this would work from macro-expanded code, see if that's feasible to generate (goalc can surely do this...somehow...right?)
#  - in any case, this means that potentially we are missing a dependency because the expanded macro is what references something

# Steps:
# Use the symbol map to statically analyze the code base to figure out the dependency graph
    # If this proves problematic (misses or false-positives) then move it into goalc as part of the compilation process
    # can fwd decls be eliminated as a result, are they only currently there because of the jank ass build order
        # we can determine this by generating a graph where we ignore decls, if there are disconnected nodes or cycles -- we have our answer.
# Dependency graph can be used to insert imports
# Once this is made once, it can be manually maintained (if new files are added)
# Compare the current build order with the auto-generated one

import glob
from graphlib import TopologicalSorter
import json
import subprocess

class SetEncoder(json.JSONEncoder):
    def default(self, obj):
        if isinstance(obj, set):
            return list(obj)
        return json.JSONEncoder.default(self, obj)

def transitive_reduction(graph):
    nodes = list(graph.keys())
    n = len(nodes)
    indices = {nodes[i]: i for i in range(n)}
    adjacency_matrix = [[0] * n for _ in range(n)]

    # Create adjacency matrix
    for i, node in enumerate(nodes):
        for neighbor in graph[node]:
            adjacency_matrix[i][indices[neighbor]] = 1

    # Floyd-Warshall algorithm
    for k in range(n):
        for i in range(n):
            for j in range(n):
                if adjacency_matrix[i][k] and adjacency_matrix[k][j]:
                    adjacency_matrix[i][j] = 0

    # Convert adjacency matrix back to graph
    for i, node in enumerate(nodes):
        graph[node] = {nodes[j] for j in range(n) if adjacency_matrix[i][j]}

    return graph

# Read in symbols
with open('./jak1-symbol-map.json', 'r') as f:
    symbol_map = json.load(f)

# TODO - hate this, this is something that can be cleaned up (referencing other code)
dep_graph = {'goal-lib.gc': set()}
dep_graph_reasons = {'goal-lib.gc': []}

source_files = glob.glob('../../../goal_src/jak1/**/*.gc', recursive=True)

for i, source_file in enumerate(source_files):
    contents = open(source_file, 'r').read()
    source_file_name = source_file.split('goal_src/jak1\\')[1].replace("\\", "/")
    # ensure that every file exists in the graph, even if it ends up having no deps
    if source_file_name not in dep_graph:
        dep_graph[source_file_name] = set()
    print('Processing {} of {}'.format(i, len(source_files)))
    for symbol_name, symbol_info in symbol_map.items():
        # functions
        add_symbol = False
        if symbol_info['kind'] == 2 or symbol_info['kind'] == 5:
            add_symbol = f"({symbol_name} " in contents
        elif symbol_info['kind'] == 0 or symbol_info['kind'] == 4:
            # NOTE - yes this means vars as the head of a form is missed, assuming thats ok, this doesn't need to be perfect
            add_symbol = f" {symbol_name} " in contents
        elif symbol_info['kind'] == 3:
            # NOTE - yes this means vars as the head of a form is missed, assuming thats ok, this doesn't need to be perfect
            add_symbol = f" {symbol_name} " in contents or f" {symbol_name})" in contents
        if add_symbol:
            if source_file_name not in dep_graph:
                dep_graph[source_file_name] = set()
            if source_file_name not in dep_graph_reasons:
                dep_graph_reasons[source_file_name] = []
            if "def_location" in symbol_info:
                # skip if its defined in the same file
                if source_file_name == symbol_info["def_location"]["filename"]:
                    continue
                dep_graph[source_file_name].add(symbol_info["def_location"]["filename"])
                dep_graph_reasons[source_file_name].append(symbol_name)
                    

with open('graph.json', 'w') as f:
    f.write(json.dumps(dep_graph, indent=2, cls=SetEncoder))

reduced_graph = transitive_reduction(dep_graph)

with open('reduced_graph.json', 'w') as f:
    f.write(json.dumps(reduced_graph, indent=2, cls=SetEncoder))

with open('dep_graph_reasons.json', 'w') as f:
    f.write(json.dumps(dep_graph_reasons, indent=2, cls=SetEncoder))

# For debugging (and interest) convert the graph into a mermaid representation
# Generate a topological ordering so the graph will actually be somewhat top-down
ts = TopologicalSorter(reduced_graph)
topo_order = tuple(ts.static_order())

with open('topo_order.txt', 'w') as f:
    f.write('\n'.join(topo_order))