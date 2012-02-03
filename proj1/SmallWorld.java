/*
  CS 61C Project1: Small World

  Name: Hairan Zhu
  Login: cs61c-eu

  Name: Benjamin Han
  Login: cs61c-mm
 */

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.DataInput;
import java.io.DataOutput;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.lang.Math;
import java.util.*;

import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.filecache.DistributedCache;
import org.apache.hadoop.fs.FileSystem;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.io.Text;
import org.apache.hadoop.io.LongWritable;
import org.apache.hadoop.io.Writable;
import org.apache.hadoop.mapreduce.Job;
import org.apache.hadoop.mapreduce.Mapper;
import org.apache.hadoop.mapreduce.Reducer;
import org.apache.hadoop.mapreduce.lib.input.FileInputFormat;
import org.apache.hadoop.mapreduce.lib.input.TextInputFormat;
import org.apache.hadoop.mapreduce.lib.input.SequenceFileInputFormat;
import org.apache.hadoop.mapreduce.lib.output.FileOutputFormat;
import org.apache.hadoop.mapreduce.lib.output.TextOutputFormat;
import org.apache.hadoop.mapreduce.lib.output.SequenceFileOutputFormat;
import org.apache.hadoop.util.GenericOptionsParser;

public class SmallWorld {

	// Maximum dept for any breadth-first search
	public static final int MAX_ITERATIONS = 5;

	// Skeleton code uses this to share denom cmd-line arg across cluster
	public static final String DENOM_PATH = "denom.txt";

	// Example enumerated type, used by EValue and Counter example
	// The number of starting vertices
	public static enum ValueUse {
		DESTINATION, DISTANCE
	};

	// Example writable type
	public static class EValue implements Writable {

		/** What this value represents. */
		public ValueUse type;
		/** The first value. Either destination or distance.*/
		public long value1;
		/** The second value. If type == DISTANCE, this is origin id. */
		public long value2;

		public EValue(ValueUse type, long value) {
			this.type = type;
			this.value1 = value;
			this.value2 = -1;
		}

		public EValue(ValueUse type, long value1, long value2) {
			this.type = type;
			this.value1 = value1;
			this.value2 = value2;
		}

		// Serializes object - needed for Writable
		public void write(DataOutput out) throws IOException {
			out.writeUTF(type.name());
			out.writeLong(value1);
			out.writeLong(value2);
		}

		// Deserializes object - needed for Writable
		public void readFields(DataInput in) throws IOException {
			type = ValueUse.valueOf(in.readUTF());
			value1 = in.readLong();
			value2 = in.readLong();
		}

		public void set(ValueUse use, long value) {
			this.type = use;
			this.value1 = value;
		}

		public String toString() {
			return type.name() + ": " + value1;
		}

		public long getValue1() {
			return value1;
		}

		public long getValue2() {
			return value2;
		}

		public ValueUse getType() {
			return type;
		}
	}

	/*
	 * This example mapper loads in all edges but only propagates a subset. You
	 * will need to modify this to propagate all edges, but it is included to
	 * demonstate how to read & use the denom argument.
	 */
	public static class LoaderMap extends
			Mapper<LongWritable, LongWritable, LongWritable, EValue> {
		public long denom;

		/*
		 * Setup is called automatically once per map task. This will read denom
		 * in from the DistributedCache, and it will be available to each call
		 * of map later on via the instance variable.
		 */
		@Override
		public void setup(Context context) {
			try {
				Configuration conf = context.getConfiguration();
				Path cachedDenomPath = DistributedCache
						.getLocalCacheFiles(conf)[0];
				BufferedReader reader = new BufferedReader(new FileReader(
						cachedDenomPath.toString()));
				String denomStr = reader.readLine();
				reader.close();
				denom = Long.decode(denomStr);
			} catch (IOException ioe) {
				System.err
						.println("IOException reading denom from distributed cache");
				System.err.println(ioe.toString());
			}
		}

		/* Will need to modify to not lose any edges. */
		@Override
		public void map(LongWritable key, LongWritable value, Context context)
				throws IOException, InterruptedException {
			// Send edge forward only if part of random subset
			long sourceId = value.get();
			context.write(key, new EValue(ValueUse.DESTINATION, sourceId));
			if (Math.random() < 1.0 / denom) {
				value = new LongWritable(-1);
				context.write(key, new EValue(ValueUse.DISTANCE, 0, sourceId));
			} else {
				context.write(key, new EValue(ValueUse.DISTANCE,
						MAX_ITERATIONS + 1));
			}
		}
	}

	/** Does nothing. */
	public static class LoadReduce extends
			Reducer<LongWritable, EValue, LongWritable, EValue> {

		/** Identity reduce. */
		@Override
		public void reduce(LongWritable key, Iterable<EValue> values,
				Context context) throws IOException, InterruptedException {
			for (EValue val : values) {
				context.write(key, val);
			}
		}
	}

	/*
	 * Insert your mapreduces here (still feel free to edit elsewhere)
	 */

	// Shares denom argument across the cluster via DistributedCache
	public static void shareDenom(String denomStr, Configuration conf) {
		try {
			Path localDenomPath = new Path(DENOM_PATH + "-source");
			Path remoteDenomPath = new Path(DENOM_PATH);
			BufferedWriter writer = new BufferedWriter(new FileWriter(
					localDenomPath.toString()));
			writer.write(denomStr);
			writer.newLine();
			writer.close();
			FileSystem fs = FileSystem.get(conf);
			fs.copyFromLocalFile(true, true, localDenomPath, remoteDenomPath);
			DistributedCache.addCacheFile(remoteDenomPath.toUri(), conf);
		} catch (IOException ioe) {
			System.err.println("IOException writing to distributed cache");
			System.err.println(ioe.toString());
		}
	}

	/** Does nothing. */
	public static class SearchMap extends
			Mapper<LongWritable, EValue, LongWritable, EValue> {
		@Override
		public void map(LongWritable key, EValue value, Context context)
				throws IOException, InterruptedException {
			context.write(key, value);
		}
	}

	/** Propagation. */
	public static class SearchReduce extends
			Reducer<LongWritable, EValue, LongWritable, EValue> {

		/** Identity reduce. */
		@Override
		public void reduce(LongWritable key, Iterable<EValue> values,
				Context context) throws IOException, InterruptedException {
			ArrayList<Long> destinations = new ArrayList<Long>();
			HashMap<Long, Long> distances = new HashMap<Long, Long>();
			for (EValue val : values) {
				if (val.getType() == ValueUse.DESTINATION) {
					context.write(key, val); // propagate graph
					destinations.add(val.getValue1());
				} else if (val.getType() == ValueUse.DISTANCE) {
					// take minimum distance ea. time
					long origin = val.getValue1();
					long distance = val.getValue1();
					if (distances.containsKey(origin)) {
						if (distances.get(origin) > distance) {
							distances.put(origin, distance);
						}
					} else {
						distances.put(origin, distance);
					}
				}
			}
			// update distance for successors
			for (EValue val : values) {
				if (val.getType() == ValueUse.DISTANCE) {
					for (long destination : destinations) {
						context.write(key, new EValue(ValueUse.DISTANCE,
								distances.get(val.getValue1()), destination));
					}
				}
			}
		}
	}

	/** Just keep distance information for reachable nodes. */
	public static class CleanupMap extends
			Mapper<LongWritable, EValue, LongWritable, EValue> {
		@Override
		public void map(LongWritable key, EValue value, Context context)
				throws IOException, InterruptedException {
			if (value.getType() == ValueUse.DISTANCE
					&& value.getValue1() <= MAX_ITERATIONS) {
				context.write(key, value);
			}
		}
	}

	/** Output origin and distance. */
	public static class CleanupReduce extends
			Reducer<LongWritable, EValue, LongWritable, LongWritable> {
		public void reduce(LongWritable key, Iterable<EValue> values,
				Context context) throws IOException, InterruptedException {
			HashMap<Long, Long> distances = new HashMap<Long, Long>();
			for (EValue val : values) {
				if (val.getType() == ValueUse.DISTANCE) {
					// take minimum distance ea. time
					long origin = val.getValue1();
					long distance = val.getValue1();
					if (distances.containsKey(origin)) {
						if (distances.get(origin) > distance) {
							distances.put(origin, distance);
						}
					} else {
						distances.put(origin, distance);
					}
				}
			}
			for (EValue val : values) {
				context.write(new LongWritable(val.getValue2()), new LongWritable(val.getValue1()));
			}
		}
	}

	/** Do nothing. */
	public static class HistogramMap extends
			Mapper<LongWritable, EValue, LongWritable, EValue> {
		@Override
		public void map(LongWritable key, EValue value, Context context)
				throws IOException, InterruptedException {
			if (value.getType() == ValueUse.DISTANCE
					&& value.getValue1() <= MAX_ITERATIONS) {
				context.write(key, value);
			}
		}
	}

	/** Make the histogram. Probably have to change output to a list of longs? */
	public static class HistogramReduce extends
	Reducer<LongWritable, LongWritable, LongWritable, Text> {
		public void reduce(LongWritable key, Iterable<LongWritable> values,
				Context context) throws IOException, InterruptedException {
			long[] counts = new long[MAX_ITERATIONS];
			for (LongWritable val : values) {
				counts[(int) val.get()] += 1; //do the counting
			}
			context.write(key, new Text(Arrays.toString(counts)));
		}
	}

	public static void main(String[] rawArgs) throws Exception {
		GenericOptionsParser parser = new GenericOptionsParser(rawArgs);
		Configuration conf = parser.getConfiguration();
		String[] args = parser.getRemainingArgs();

		// Set denom from command line arguments
		shareDenom(args[2], conf);

		// Setting up mapreduce job to load in graph
		Job job = new Job(conf, "load graph");
		job.setJarByClass(SmallWorld.class);

		job.setMapOutputKeyClass(LongWritable.class);
		job.setMapOutputValueClass(EValue.class);
		job.setOutputKeyClass(LongWritable.class);
		job.setOutputValueClass(EValue.class);

		job.setMapperClass(LoaderMap.class);
		job.setReducerClass(LoadReduce.class);

		job.setInputFormatClass(SequenceFileInputFormat.class);
		job.setOutputFormatClass(SequenceFileOutputFormat.class);

		// Input from command-line argument, output to predictable place
		FileInputFormat.addInputPath(job, new Path(args[0]));
		FileOutputFormat.setOutputPath(job, new Path("bfs-0-out"));

		// Actually starts job, and waits for it to finish
		job.waitForCompletion(true);

		// Example of reading a counter
		/*
		 * System.out.println("Read in " +
		 * job.getCounters().findCounter(ValueUse.EDGE).getValue() + " edges");
		 */

		// Repeats your BFS mapreduce
		int i = 0;
		// Will need to change terminating conditions to respond to data
		while (i < MAX_ITERATIONS) {
			job = new Job(conf, "bfs" + i);
			job.setJarByClass(SmallWorld.class);

			job.setMapOutputKeyClass(LongWritable.class);
			job.setMapOutputValueClass(EValue.class);
			job.setOutputKeyClass(LongWritable.class);
			job.setOutputValueClass(EValue.class);

			job.setMapperClass(SearchMap.class);
			job.setReducerClass(SearchReduce.class);

			job.setInputFormatClass(SequenceFileInputFormat.class);
			job.setOutputFormatClass(SequenceFileOutputFormat.class);

			// Notice how each mapreduce job gets gets its own output dir
			FileInputFormat.addInputPath(job, new Path("bfs-" + i + "-out"));
			FileOutputFormat.setOutputPath(job, new Path("bfs-" + (i + 1)
					+ "-out"));

			job.waitForCompletion(true);
			i++;
		}

		// Cleanup
		job = new Job(conf, "cleanup");
		job.setJarByClass(SmallWorld.class);

		job.setMapOutputKeyClass(LongWritable.class);
		job.setMapOutputValueClass(EValue.class);
		job.setOutputKeyClass(LongWritable.class);
		job.setOutputValueClass(LongWritable.class);

		job.setMapperClass(CleanupMap.class);
		job.setReducerClass(CleanupReduce.class);

		job.setInputFormatClass(SequenceFileInputFormat.class);
		job.setOutputFormatClass(SequenceFileOutputFormat.class);

		// By declaring i above outside of loop conditions, can use it
		// here to get last bfs output to be input to histogram
		FileInputFormat.addInputPath(job, new Path("bfs-" + i + "-out"));
		i++;
		FileOutputFormat.setOutputPath(job, new Path("bfs-" + i + "-out"));

		job.waitForCompletion(true);


		// Mapreduce config for histogram computation
		job = new Job(conf, "hist");
		job.setJarByClass(SmallWorld.class);

		job.setMapOutputKeyClass(LongWritable.class);
		job.setMapOutputValueClass(LongWritable.class);
		job.setOutputKeyClass(LongWritable.class);
		job.setOutputValueClass(Text.class);

		job.setMapperClass(HistogramMap.class);
		job.setReducerClass(HistogramReduce.class);

		job.setInputFormatClass(SequenceFileInputFormat.class);
		job.setOutputFormatClass(TextOutputFormat.class);

		// By declaring i above outside of loop conditions, can use it
		// here to get last bfs output to be input to histogram
		FileInputFormat.addInputPath(job, new Path("bfs-" + i + "-out"));
		FileOutputFormat.setOutputPath(job, new Path(args[1]));

		job.waitForCompletion(true);
	}
}
