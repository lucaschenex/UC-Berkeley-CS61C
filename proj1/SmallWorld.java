/*
  CS 61C Project1: Small World
  2/4: 11:15

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

	/** Maximum depth for any breadth-first search. */
	public static final int MAX_ITERATIONS = 10; //SHOULD BE 20
	/** An unreachable distance. */
	public static final int MAX_DISTANCE = MAX_ITERATIONS + 1;

	// Skeleton code uses this to share denom cmd-line arg across cluster
	public static final String DENOM_PATH = "denom.txt";

    /** The number of starting points. */
    public static int _origins = 0;

    /** The number of starting points. */
    public static enum GraphCounter {
	ORIGINS};

	/** What type of value. */
	public static enum ValueUse {
    	        DESTINATION, DISTANCE, BLANK
	};

	/** Stores information for each vertex in the graph. */
	public static class EValue implements Writable {

		/** What this value represents. */
		public ValueUse type;
		/** The first value. Either destination or distance.*/
		public long destDist;
		/** The second value. If type == DISTANCE, this is origin id. */
		public long origin;

		/** Hadoop requires a default constructor. */
		public EValue() {
			this(ValueUse.BLANK, -1, -1);
		}

		/** For destination type. */
		public EValue(ValueUse type, long value) {
			this(type, value, -1);
		}

		/** For distance type. */
		public EValue(ValueUse type, long value1, long value2) {
			this.type = type;
			this.destDist = value1;
			this.origin = value2;
		}

		// Serializes object - needed for Writable
		public void write(DataOutput out) throws IOException {
			out.writeUTF(type.name());
			out.writeLong(destDist);
			out.writeLong(origin);
		}

		// Deserializes object - needed for Writable
		public void readFields(DataInput in) throws IOException {
			type = ValueUse.valueOf(in.readUTF());
			destDist = in.readLong();
			origin = in.readLong();
		}

		/** Returns distance or destination. */
		public long getDistDest() {
			return destDist;
		}

		/** Returns origin. */
		public long getOrigin() {
			return origin;
		}

		/** Returns type. */
		public ValueUse getType() {
			return type;
		}
	}

	/** Loads map. */
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

		/* Select vertices as starting points with probability 1/denom.
		 * Those get marked with distance 0. */
		@Override
		public void map(LongWritable key, LongWritable value, Context context)
				throws IOException, InterruptedException {
			context.write(key, new EValue(ValueUse.DESTINATION, value.get())); //propagate graph
			if (Math.random() < 1.0 / denom) {
				// 0 distance will help the next mapreduce know where to start.
			        context.getCounter(ValueUse.origins).increment(1);
				context.write(key, new EValue(ValueUse.DISTANCE, 0, key.get()));
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

	/** Propagation of breadth-first search. */
	public static class SearchReduce extends
			Reducer<LongWritable, EValue, LongWritable, EValue> {

		/* Updates distances. Probably VERY INEFFICIENT right now.
		 * I'm adding one to the appropriate distance of each successor.
		 * For example:
		 * 
		 * A -> B -> C -> D
		 * 
		 * B is distance 1 from A. So we know that C must be 1 more away from A than B.
		 * 
		 * The problem is that this gets called each time. I haven't implemented a check
		 * that says the distance of C from A has been found -- that we can stop.
		 * I can't think of a better way. This means right now though that after ea. iteration,
		 * a pair of points say (E, F) might have several distances say 4, 8, 10, 12. The purpose
		 * of the HashMap is to store the minimum distance each time. */
		@Override
		public void reduce(LongWritable key, Iterable<EValue> values,
				Context context) throws IOException, InterruptedException {
			// keeps track of all successors for this vertex (key)
			ArrayList<Long> destinations = new ArrayList<Long>();
			//distances maps from ea. origin to distance(key, origin)
			HashMap<Long, Long> distances = new HashMap<Long, Long>();
			// calculate min. distances
			for (EValue val : values) {
				if (val.getType() == ValueUse.DESTINATION) {
				    //context.write(key, val); //propagate destination
					destinations.add(val.getDistDest());
				} else if (val.getType() == ValueUse.DISTANCE) {
					// take minimum distance ea. time
					long origin = val.getOrigin();
					long distance = val.getDistDest();
					if (distances.containsKey(origin)) {
						if (distances.get(origin) > distance) {
							distances.put(origin, distance);
						}
					} else {
						distances.put(origin, distance);
					}
				}
			}
			// write updated distances
			for (Map.Entry<Long, Long> pairing : distances.entrySet()) {
				context.write(key,
						new EValue(ValueUse.DISTANCE, pairing.getValue(), pairing.getKey()));
			}
			/* update distance for successors, one more than current
			 * emit a pair (destination, DISTANCE distance+1 origin)
			 * done only if current vertex has a distance value from that origin
			 */
			for (long origin : distances.keySet()) {
				long distance = distances.get(origin);
				for (long destination : destinations) {
					context.write(new LongWritable(destination),
							new EValue(ValueUse.DISTANCE, distance + 1, origin));
				}
			}
			if (distances.size() < _origins) {
			    for (long dest : destinations) {
				context.write(key, new Evalue(ValueUse.DESTINATION, dest));
			    }
			}	
		}
	}

	/** Just keep distance information. */
	public static class CleanupMap extends
			Mapper<LongWritable, EValue, LongWritable, EValue> {
		@Override
		public void map(LongWritable key, EValue value, Context context)
				throws IOException, InterruptedException {
			if (value.getType() == ValueUse.DISTANCE) {
				context.write(key, value);
			}
		}
	}

	/** Output (distance, 1) for each origin of each key. */
	public static class CleanupReduce extends
			Reducer<LongWritable, EValue, LongWritable, LongWritable> {

		public static LongWritable ONE = new LongWritable(1L);

		@Override
		public void reduce(LongWritable key, Iterable<EValue> values,
				Context context) throws IOException, InterruptedException {
			//distances maps from ea. origin to distance of this key from that origin
			HashMap<Long, Long> distances = new HashMap<Long, Long>();
			for (EValue val : values) {
				// take minimum distance ea. time
				long origin = val.getOrigin();
				long distance = val.getDistDest();
				if (distances.containsKey(origin)) {
					if (distances.get(origin) > distance) {
						distances.put(origin, distance);
					}
				} else {
					distances.put(origin, distance);
				}
			}
			for (Long distance : distances.values()) {
				context.write(new LongWritable(distance), ONE);
			}
		}
	}

	/** Do nothing. */
	public static class HistogramMap extends
			Mapper<LongWritable, LongWritable, LongWritable, LongWritable> {
		@Override
		public void map(LongWritable key, LongWritable value, Context context)
				throws IOException, InterruptedException {
			context.write(key, value);
		}
	}

	/** Make the histogram. Probably have to change output to a list of longs? */
	public static class HistogramReduce extends
	Reducer<LongWritable, LongWritable, LongWritable, Text> {
		@Override
		public void reduce(LongWritable key, Iterable<LongWritable> values,
				Context context) throws IOException, InterruptedException {
			long sum = 0L;
			for (LongWritable val : values) {
				sum += 1; //do the counting
			}
			context.write(key, new Text(Long.toString(sum)));
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

		// Loads number of origins
		_origins = job.getCounters().findCounter(GraphCounter.ORIGINS).getValue();

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

		FileInputFormat.addInputPath(job, new Path("bfs-" + i + "-out"));
		FileOutputFormat.setOutputPath(job, new Path("bfs-" + (i + 1) + "-out"));

		job.waitForCompletion(true);
		i++;
		

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
