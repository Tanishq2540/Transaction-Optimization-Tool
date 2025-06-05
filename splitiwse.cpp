#include <bits/stdc++.h>
using namespace std;
using namespace std::chrono;

struct Transaction {
    int from, to;
    int amount;
};

vector<long long> getNetBalances(const vector<Transaction> &transactions, int n) {
    vector<long long> net(n, 0);
    for (auto &t : transactions) {
        net[t.from] -= t.amount;
        net[t.to] += t.amount;
    }
    return net;
}

// Helper to find the minimum balance (debtor)
pair<long long, int> findMin(const vector<long long>& amt) {
    int n = amt.size();
    long long mini = 0;
    int debitor = 0;
    for (int i = 0; i < n; i++) {
        if (amt[i] <= mini) {
            mini = amt[i];
            debitor = i;
        }
    }
    return {mini, debitor};
}

// Helper to find the maximum balance (creditor)
pair<long long, int> findMax(const vector<long long>& amt) {
    int n = amt.size();
    long long maxi = 0;
    int creditor = 0;
    for (int i = 0; i < n; i++) {
        if (amt[i] >= maxi) {
            maxi = amt[i];
            creditor = i;
        }
    }
    return {maxi, creditor};
}

// Replaced greedy function using findMin and findMax
int greedyMinTransactions(vector<long long> net) {
    int cnt = 0;
    while (true) {
        auto [mini, debitor] = findMin(net);
        auto [maxi, creditor] = findMax(net);
        
        if (mini == 0 && maxi == 0) break;
        
        long long settled = min(abs(mini), maxi);
        net[debitor] += settled;
        net[creditor] -= settled;
        cnt++;
    }
    return cnt;
}

int backtrackingUtil(vector<long long> &net, int start) {
    while (start < (int)net.size() && net[start] == 0) start++;
    if (start == (int)net.size()) return 0;
    
    int minTrans = INT_MAX;
    for (int i = start + 1; i < (int)net.size(); i++) {
        if (net[start] * net[i] < 0) {
            net[i] += net[start];
            minTrans = min(minTrans, 1 + backtrackingUtil(net, start + 1));
            net[i] -= net[start];
        }
    }
    return minTrans;
}

int backtrackingMinTransactions(vector<long long> net) {
    return backtrackingUtil(net, 0);
}

int dpBitmaskMinTransactions(vector<long long> &net) {
    vector<long long> debts;
    for (long long x : net) 
        if (x != 0) debts.push_back(x);

    int n = debts.size();
    if (n == 0) return 0;

    vector<long long> sum(1 << n, 0);
    for (int mask = 1; mask < (1 << n); mask++) {
        int lsb = mask & -mask;
        int idx = __builtin_ctz(lsb);
        sum[mask] = sum[mask ^ lsb] + debts[idx];
    }

    vector<int> dp(1 << n, INT_MAX);
    dp[0] = 0;

    for (int mask = 1; mask < (1 << n); mask++) {
        if (sum[mask] == 0) {
            dp[mask] = __builtin_popcount(mask) - 1;
            for (int sub = (mask - 1) & mask; sub > 0; sub = (sub - 1) & mask) {
                if (dp[sub] != INT_MAX && dp[mask ^ sub] != INT_MAX) {
                    dp[mask] = min(dp[mask], dp[sub] + dp[mask ^ sub]);
                }
            }
        }
    }

    return dp[(1 << n) - 1];
}

vector<Transaction> generateRandomTransactions(int n, int numTransactions, int maxAmount) {
    vector<Transaction> trans;
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> distP(0, n - 1);
    uniform_int_distribution<> distAmt(1, maxAmount);

    for (int i = 0; i < numTransactions; i++) {
        int from = distP(gen);
        int to = distP(gen);
        while (to == from) to = distP(gen);
        int amount = distAmt(gen);
        trans.push_back({from, to, amount});
    }
    return trans;
}

tuple<int, int, int, double, double, double> runTestCase(const vector<Transaction> &transactions, int n) {
    auto net = getNetBalances(transactions, n);

    auto start = high_resolution_clock::now();
    int greedyAns = greedyMinTransactions(net);
    auto end = high_resolution_clock::now();
    double greedyTime = duration_cast<microseconds>(end - start).count();

    start = high_resolution_clock::now();
    int backAns = backtrackingMinTransactions(net);
    end = high_resolution_clock::now();
    double backTime = duration_cast<microseconds>(end - start).count();

    start = high_resolution_clock::now();
    int dpAns = dpBitmaskMinTransactions(net);
    end = high_resolution_clock::now();
    double dpTime = duration_cast<microseconds>(end - start).count();

    return {greedyAns, backAns, dpAns, greedyTime, backTime, dpTime};
}

void exportCSV(const vector<tuple<int,int,int,double,double,double,int>> &data, string filename) {
    ofstream fout(filename);
    fout << "n,NumTransactions_Greedy,NumTransactions_Backtracking,NumTransactions_DP,Time_Greedy_us,Time_Backtracking_us,Time_DP_us\n";
    for (auto &row : data) {
        int n, g, b, d;
        double gt, bt, dt;
        tie(g, b, d, gt, bt, dt, n) = row;
        fout << n << "," << g << "," << b << "," << d << "," << gt << "," << bt << "," << dt << "\n";
    }
    fout.close();
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    cout << "Splitwise Debt Settlement Benchmark\n";
    cout << "Running tests...\n";

    vector<tuple<int,int,int,double,double,double,int>> results;

    for (int n = 4; n <= 12; n ++) {
        int numTransactions = n * 3;
        int maxAmount = 100;

        auto transactions = generateRandomTransactions(n, numTransactions, maxAmount);
        cout << "Test n=" << n << " with " << numTransactions << " transactions\n";

        int REPEAT = 1000;
        double sumGreedy = 0, sumBack = 0, sumDP = 0;
        int greedyAns = -1, backAns = -1, dpAns = -1;

        for (int i = 0; i < REPEAT; i++) {
            auto [g, b, d, gt, bt, dt] = runTestCase(transactions, n);
            if (greedyAns == -1) {
                greedyAns = g; backAns = b; dpAns = d;
            }
            sumGreedy += gt; sumBack += bt; sumDP += dt;
        }

        results.push_back({greedyAns, backAns, dpAns, sumGreedy/REPEAT, sumBack/REPEAT, sumDP/REPEAT, n});
        cout << "Done n=" << n << "\n";
    }

    cout<<"exporting"<<endl;
    exportCSV(results, "splitwise_benchmark.csv");
    cout << "Results exported to splitwise_benchmark.csv\n";

    return 0;
}
