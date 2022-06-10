"""
A benchmark suite for Performance Anti-Patterns
"""
import argparse
import timeit
import pathlib
from types import FunctionType
import pyinstrument
from statistics import fmean
from rich.console import Console
from rich.table import Table
from rich.text import Text

REPEAT = 5
TIMES = 5

def format_delta(a: float, b: float, d: float) -> Text:
    if a < b:
        if d < 10:
            col = "medium_spring_green"
        elif 10 <= d < 20:  
            col = "spring_green1"
        elif 20 <= d < 40:
            col = "spring_green2"
        else:
            col = "green1"
        return Text(f"{a:.3f} ({d:.1f}%)", style=col)
    else:
        return Text(f"{a:.3f} (-{d:.1f}%)", style="red")


def benchmark_function(func: FunctionType, profile=False):
    if profile:
        profiles_out = pathlib.Path(__file__).parent / '.profiles'
        if not profiles_out.exists():
            profiles_out.mkdir(parents=True)
        profiler = pyinstrument.Profiler()
        profiler.start()

    result = timeit.repeat(func, repeat=REPEAT, number=TIMES)

    if profile:
        profiler.stop()
        with open(profiles_out / f"{func.__name__}.html", "w", encoding='utf-8') as html:
            html.write(profiler.output_html())
    
    return result


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument('--profile', action='store_true')
    parser.add_argument('benchmark', nargs='?', default=None)
    args = parser.parse_args()

    table = Table(title=f"Picologging Benchmark Suite, repeat={REPEAT}, number={TIMES}")

    table.add_column("Benchmark", justify="right", style="cyan", no_wrap=True)
    table.add_column("Min", width=7)
    table.add_column("Max", width=7)
    table.add_column("Mean", width=7)
    table.add_column("Min (+)", style="blue", width=15)
    table.add_column("Max (+)", style="blue", width=15)
    table.add_column("Mean (+)", style="blue", width=15)

    n = 0

    for f in pathlib.Path(__file__).parent.glob("bench_*.py"):
        if args.benchmark and f.stem != f"bench_{args.benchmark}":
            continue
        i = __import__(f.stem, globals(), locals(), )
        if hasattr(i, "__benchmarks__"):
            for benchmark in i.__benchmarks__:
                n += 1
                func1, func2, desc = benchmark
                
                without_result = benchmark_function(func1, args.profile)
                with_result = benchmark_function(func2, args.profile)

                delta_mean = (abs(fmean(with_result) - fmean(without_result)) / fmean(without_result)) * 100.0
                delta_min = (abs(min(with_result) - min(without_result)) / min(without_result)) * 100.0
                delta_max = (abs(max(with_result) - max(without_result)) / max(without_result)) * 100.0

                fdelta_min = format_delta(min(with_result), min(without_result), delta_min)
                fdelta_max = format_delta(max(with_result), max(without_result), delta_max)
                fdelta_mean = format_delta(fmean(with_result), fmean(without_result), delta_mean)

                table.add_row(
                              desc,
                              "{:.3f}".format(min(without_result)),
                              "{:.3f}".format(max(without_result)),
                              "{:.3f}".format(fmean(without_result)),
                              fdelta_min,
                              fdelta_max,
                              fdelta_mean,
                              )

    console = Console(width=150)
    console.print(table)