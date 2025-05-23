import matplotlib.pyplot as plt

processes = [1, 2, 4, 8]
qsort_time = [1.02] * len(processes) 
mpi_time = [2.02, 2.83, 3.01, 3.22] 

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

plt.tight_layout()
plt.savefig("sort_comparison1.png")
plt.show()
