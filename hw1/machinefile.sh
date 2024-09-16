#!/bin/bash

# 获取当前作业的节点列表
nodes=$(scontrol show hostnames $SLURM_JOB_NODELIST)

# 生成 machinefile
for node in $nodes; do
    echo "$node" >> machinefile
done

echo "machinefile generated successfully."
