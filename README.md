# Earnings Impact Study

This project analyzes how stocks perform around earnings announcement dates, grouping them by whether they beat, meet, or miss earnings expectations.

## Features

- **Earnings Analysis**: Groups stocks into Beat, Meet, and Miss categories based on EPS surprise percentages
- **Event Window Calculation**: Computes abnormal returns around earnings announcement dates
- **Statistical Analysis**: Calculates Average Abnormal Returns (AAR) and Cumulative Average Abnormal Returns (CAAR)
- **Bootstrapping**: Statistical validation through bootstrap resampling
- **Benchmark Comparison**: Uses Russell 3000 ETF (IWV) as market benchmark
- **Data Visualization**: Gnuplot integration for result visualization
- **API Integration**: Fetches historical price data from EODHD API

## Prerequisites

- **C++ Compiler**: g++ (MinGW-w64 on Windows)
- **Python 3.x**: For data preparation notebooks
- **Libraries**:
  - libcurl: For API calls to EODHD
  - Standard C++ libraries
- **EODHD API Key**: Required for fetching historical stock prices

## Project Structure

```
Earnings_Impact_Study/
├── src/                          # Source implementation files
│   ├── analysis/                 # Abnormal return calculations
│   ├── common/                   # Matrix operations and utilities
│   ├── core/                     # Stock and Sector classes
│   ├── data/                     # Data loading and processing
│   └── ui_visualization/         # Menu and Gnuplot integration
├── include/                      # Header files (mirrors src structure)
├── data/                         # Datasets and preparation
│   ├── earnings_cleaned.csv      # Cleaned earnings data
│   ├── iShares-Russell-3000-ETF_fund_cleaned.csv
│   ├── CleanEarningsData.ipynb   # Data cleaning notebook
│   ├── ScrapeEarningsData.ipynb  # Data scraping notebook
│   └── requirements.txt          # Python dependencies
├── build/                        # Compiled outputs and results
├── results/                      # Analysis results
│   ├── logs.txt                  # Execution logs
│   └── Results_Summary.txt       # Event window analysis results
├── build.bat                     # Windows build script
├── Makefile                      # Alternative build configuration
└── api_key.txt                   # EODHD API key
```

## Installation & Setup

### 1. Clone the Repository
```bash
git clone <repository-url>
cd Earnings_Impact_Study
```

### 2. Obtain EODHD API Key
1. Sign up at [EODHD](https://eodhd.com/)
2. Copy your API key
3. Create `api_key.txt` in the project root and paste your key.

### 3. Prepare Data (Optional)
If starting fresh, run the Python notebooks to prepare data.
The project includes pre-cleaned CSV files, so this step is optional.

## Building & Running

### Using build.bat (Windows)
```bash
.\build.bat         # Build
.\build.bat run     # Build and run
.\build.bat clean   # Clean artifacts
```

### Using Makefile (Cross-platform)
```bash
make              # Build
make run          # Build and run
make clean        # Clean artifacts
```

### Manual Compilation
```bash
g++ -I./include -g -Wall -mconsole ^
    src/ui_visualization/Menu.cpp ^
    src/common/utils.cpp ^
    src/common/Matrix.cpp ^
    src/core/Stock.cpp ^
    src/core/Sector.cpp ^
    src/data/DataLoader.cpp ^
    src/data/StockHistory.cpp ^
    src/analysis/Calculation.cpp ^
    src/ui_visualization/Gnuplot.cpp ^
    -o build/earnings_impact_study.exe -lcurl
```

## Data Files

### earnings_cleaned.csv
Contains cleaned earnings announcement data with columns:
- Ticker: Stock symbol
- Announcement Date: Date of earnings release (YYYY-MM-DD)
- Estimated EPS: Consensus estimate
- Actual EPS: Reported earnings per share
- Surprise %: (Actual - Estimated) / Estimated

### iShares-Russell-3000-ETF_fund_cleaned.csv
Contains information on stocks in the Russell-3000 ETF, with columns:
- Ticker: Stock symbol
- Name: Full name of the company
- Sector

## How It Works

The analysis is conducted through an interactive **Menu system** that guides you through the analysis pipeline:

### 1. Data Loading
- Loads earnings data from CSV
- Loads benchmark (ETF) price data
- Validates data consistency

### 2. Stock Grouping
Stocks are categorized based on earnings surprise percentage:
- **Beat**: Positive surprise (actual > estimated)
- **Meet**: Near zero surprise (actual $\approx$ estimated)
- **Miss**: Negative surprise (actual < estimated)

### 3. Event Window Analysis
For each stock:
- Defines an event window (default: ±30 days around announcement)
- Fetches historical prices via EODHD API
- Calculates daily returns: $R_t = log(\frac{P_t - P_{t-1}}{P_{t-1}})$

### 4. Abnormal Return Calculation
- **Stock Returns**: Daily percentage change in stock price
- **Benchmark Returns**: Daily percentage change in Russell 3000 ETF
- **Abnormal Returns**: $AR_t = R_t^{stock} - R_t^{benchmark}$

### 5. Statistical Analysis
- **AAR (Average Abnormal Return)**: Mean abnormal return across all stocks in a group for each day
- **CAAR (Cumulative AAR)**: Cumulative sum of AAR over the event window
- **Bootstrapping**: Resampling analysis to validate statistical significance

### 6. Results Generation
Outputs summary statistics and visualizations showing:
- AAR and CAAR for each earnings surprise group
- Statistical significance levels
- Comparative performance across groups

## Output
- **Event window analysis results:** includes Number of valid stocks, AAR and CAAR statistics, Significance test results, and Summary metrics
- **Graph:** comparison of expected CAAR across the 3 groups
- **logs:** Processing errors and warnings

## Key Classes

- **Stock**: Represents individual stock with price history and returns data
- **Sector**: Groups stocks by industry classification
- **StockHistory**: Manages historical price data and API integration
- **DataLoader**: Loads and validates CSV data
- **Matrix/Vector**: Mathematical operations for statistical analysis
- **Calculation**: Performs abnormal return and statistical calculations

## Dependencies

- **libcurl**: HTTP library for EODHD API calls
- **C++11 or later**: Standard library features
- **Gnuplot**: Optional, for visualization (if integrated)

## Notes

- Ensure sufficient API quota with EODHD before running large analyses
- Results are appended to log files; clear them before fresh runs if needed
- Stock data requires minimum historical coverage around announcement dates
