import matplotlib.pyplot as plt

processes = [1, 2, 4, 8]
qsort_time = [1.02] * len(processes) 
mpi_time = [1.02, 0.55, 0.30, 0.18] 

speedup = [qsort_time[0] / t for t in mpi_time]

plt.figure(figsize=(10, 5))

plt.subplot(1, 2, 1)
plt.plot(processes, mpi_time, marker='o', label="MPI sort")
plt.plot(processes, qsort_time, linestyle='--', label="qsort")
plt.xlabel("Processes")
plt.ylabel("Time (sec)")
plt.title("Execution Time")
plt.legend()
plt.grid(True)

plt.subplot(1, 2, 2)
plt.plot(processes, speedup, marker='o', color='green')
plt.xlabel("Processes")
plt.ylabel("Speedup")
plt.title("Speedup of MPI sort")
plt.grid(True)

plt.tight_layout()
plt.savefig("sort_comparison.png")
plt.show()
